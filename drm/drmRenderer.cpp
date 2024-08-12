/*
 * Copyright (c) 2020, NVIDIA CORPORATION. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
 
#include "drmRenderer.h"
#include "logging.h"
#include "timespec.h"

#include <strings.h>
#include <unistd.h> // close()
#include "drmlib/drm-common.h"
#include "drmlib/common.h"
#include "drmlib/cube-tex.h"
#include "drmlib/esUtil.h"

#include "glTexture.h"

#if WITH_CUDA
#include "cudaUtility.h"
#include "cudaRGB.h"
#endif
#if WITH_OPENCL
#include "ocl_utils.h"
#include "oclUtility.h"
#endif

void MyMessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam )
{
  fprintf( stdout, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
           ( type == GL_DEBUG_TYPE_ERROR_KHR ? "** GL ERROR **" : "" ),
            type, severity, message );
}

static const struct gbm *gbm;
static const struct drm *drm;
static struct rfc rfc = {0};

static const int mNumBuffers = 2;
static int mCurBuffer = 0;
static struct drm_dumb_fb mFramebuffers[mNumBuffers];
static drmModeCrtc* mSavedCrtc = nullptr;
static bool bUseDumbBuffers = false;

#define LOG_DRM  "[drm]"

static bool b_was_pinned[mNumBuffers] = { 0 };
static void* cuda_dev_ptr[mNumBuffers] = { 0 };
// even though map/unmap takes time, latency is much better compared to keeping pointer constantly mapped
// probably because it is slower to read from it when it is handled to drm?
static bool b_persistently_mapped = false;

static bool b_textured_cube = true;

static struct {
	struct egl egl;

	GLfloat aspect;

	GLuint program;
	GLint modelviewmatrix, modelviewprojectionmatrix, normalmatrix;
	GLuint vbo;
	GLuint positionsoffset, colorsoffset, normalsoffset;

	GLuint texcoordsoffset;
	GLuint texture;
	GLuint tex;
} gl;

static struct {
	GLuint tex;
	uint32_t w,h;
	EGLSyncKHR fence;
	glTexture* glTex;
} dyn_texture;

static const GLfloat vVertices[] = {
		// front
		-1.0f, -1.0f, +1.0f,
		+1.0f, -1.0f, +1.0f,
		-1.0f, +1.0f, +1.0f,
		+1.0f, +1.0f, +1.0f,
		// back
		+1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		+1.0f, +1.0f, -1.0f,
		-1.0f, +1.0f, -1.0f,
		// right
		+1.0f, -1.0f, +1.0f,
		+1.0f, -1.0f, -1.0f,
		+1.0f, +1.0f, +1.0f,
		+1.0f, +1.0f, -1.0f,
		// left
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, +1.0f,
		-1.0f, +1.0f, -1.0f,
		-1.0f, +1.0f, +1.0f,
		// top
		-1.0f, +1.0f, +1.0f,
		+1.0f, +1.0f, +1.0f,
		-1.0f, +1.0f, -1.0f,
		+1.0f, +1.0f, -1.0f,
		// bottom
		-1.0f, -1.0f, -1.0f,
		+1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, +1.0f,
		+1.0f, -1.0f, +1.0f,
};

static const GLfloat vQuadVertices[] = {
		-0.0f, -0.0f, +0.0f,
		-0.0f, +1.0f, +0.0f,
		+1.0f, -0.0f, +0.0f,
		+1.0f, +1.0f, +0.0f,
};

GLfloat vQuadTexCoords[] = {
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 1.0f
};

static const GLfloat vColors[] = {
		// front
		0.0f,  0.0f,  1.0f, // blue
		1.0f,  0.0f,  1.0f, // magenta
		0.0f,  1.0f,  1.0f, // cyan
		1.0f,  1.0f,  1.0f, // white
		// back
		1.0f,  0.0f,  0.0f, // red
		0.0f,  0.0f,  0.0f, // black
		1.0f,  1.0f,  0.0f, // yellow
		0.0f,  1.0f,  0.0f, // green
		// right
		1.0f,  0.0f,  1.0f, // magenta
		1.0f,  0.0f,  0.0f, // red
		1.0f,  1.0f,  1.0f, // white
		1.0f,  1.0f,  0.0f, // yellow
		// left
		0.0f,  0.0f,  0.0f, // black
		0.0f,  0.0f,  1.0f, // blue
		0.0f,  1.0f,  0.0f, // green
		0.0f,  1.0f,  1.0f, // cyan
		// top
		0.0f,  1.0f,  1.0f, // cyan
		1.0f,  1.0f,  1.0f, // white
		0.0f,  1.0f,  0.0f, // green
		1.0f,  1.0f,  0.0f, // yellow
		// bottom
		0.0f,  0.0f,  0.0f, // black
		1.0f,  0.0f,  0.0f, // red
		0.0f,  0.0f,  1.0f, // blue
		1.0f,  0.0f,  1.0f  // magenta
};

static const GLfloat vNormals[] = {
		// front
		+0.0f, +0.0f, +1.0f, // forward
		+0.0f, +0.0f, +1.0f, // forward
		+0.0f, +0.0f, +1.0f, // forward
		+0.0f, +0.0f, +1.0f, // forward
		// back
		+0.0f, +0.0f, -1.0f, // backward
		+0.0f, +0.0f, -1.0f, // backward
		+0.0f, +0.0f, -1.0f, // backward
		+0.0f, +0.0f, -1.0f, // backward
		// right
		+1.0f, +0.0f, +0.0f, // right
		+1.0f, +0.0f, +0.0f, // right
		+1.0f, +0.0f, +0.0f, // right
		+1.0f, +0.0f, +0.0f, // right
		// left
		-1.0f, +0.0f, +0.0f, // left
		-1.0f, +0.0f, +0.0f, // left
		-1.0f, +0.0f, +0.0f, // left
		-1.0f, +0.0f, +0.0f, // left
		// top
		+0.0f, +1.0f, +0.0f, // up
		+0.0f, +1.0f, +0.0f, // up
		+0.0f, +1.0f, +0.0f, // up
		+0.0f, +1.0f, +0.0f, // up
		// bottom
		+0.0f, -1.0f, +0.0f, // down
		+0.0f, -1.0f, +0.0f, // down
		+0.0f, -1.0f, +0.0f, // down
		+0.0f, -1.0f, +0.0f  // down
};

GLfloat vTexCoords[] = {
		//front
		1.0f, 1.0f,
		0.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		//back
		1.0f, 1.0f,
		0.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		//right
		1.0f, 1.0f,
		0.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		//left
		1.0f, 1.0f,
		0.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		//top
		1.0f, 1.0f,
		0.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		//bottom
		1.0f, 0.0f,
		0.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
};


static const char *vertex_shader_source =
		"uniform mat4 modelviewMatrix;      \n"
		"uniform mat4 modelviewprojectionMatrix;\n"
		"uniform mat3 normalMatrix;         \n"
		"                                   \n"
		"attribute vec4 in_position;        \n"
		"attribute vec3 in_normal;          \n"
		"attribute vec4 in_color;           \n"
		"\n"
		"vec4 lightSource = vec4(2.0, 2.0, 20.0, 0.0);\n"
		"                                   \n"
		"varying vec4 vVaryingColor;        \n"
		"                                   \n"
		"void main()                        \n"
		"{                                  \n"
		"    gl_Position = modelviewprojectionMatrix * in_position;\n"
		"    vec3 vEyeNormal = normalMatrix * in_normal;\n"
		"    vec4 vPosition4 = modelviewMatrix * in_position;\n"
		"    vec3 vPosition3 = vPosition4.xyz / vPosition4.w;\n"
		"    vec3 vLightDir = normalize(lightSource.xyz - vPosition3);\n"
		"    float diff = max(0.0, dot(vEyeNormal, vLightDir));\n"
		"    vVaryingColor = vec4(diff * in_color.rgb, 1.0);\n"
		"}                                  \n";

static const char *fragment_shader_source =
		"precision mediump float;           \n"
		"                                   \n"
		"varying vec4 vVaryingColor;        \n"
		"                                   \n"
		"void main()                        \n"
		"{                                  \n"
		"    gl_FragColor = vVaryingColor;  \n"
		"}                                  \n";

static const char *vertex_shader_source_1img =
		"uniform mat4 modelviewMatrix;      \n"
		"uniform mat4 modelviewprojectionMatrix;\n"
		"uniform mat3 normalMatrix;         \n"
		"                                   \n"
		"attribute vec4 in_position;        \n"
		"attribute vec3 in_normal;          \n"
		"attribute vec2 in_TexCoord;        \n"
		"                                   \n"
		"vec4 lightSource = vec4(2.0, 2.0, 20.0, 0.0);\n"
		"                                   \n"
		"varying vec4 vVaryingColor;        \n"
		"varying vec2 vTexCoord;            \n"
		"                                   \n"
		"void main()                        \n"
		"{                                  \n"
		"    gl_Position = modelviewprojectionMatrix * in_position;\n"
		"    vec3 vEyeNormal = normalMatrix * in_normal;\n"
		"    vec4 vPosition4 = modelviewMatrix * in_position;\n"
		"    vec3 vPosition3 = vPosition4.xyz / vPosition4.w;\n"
		"    vec3 vLightDir = normalize(lightSource.xyz - vPosition3);\n"
		"    float diff = max(0.0, dot(vEyeNormal, vLightDir));\n"
		"    vVaryingColor = vec4(diff * vec3(1.0, 1.0, 1.0), 1.0);\n"
		"    vTexCoord = in_TexCoord; \n"
		"}                            \n";

static const char *fragment_shader_source_1img =
		//"#extension GL_OES_EGL_image_external : enable\n"
		"precision mediump float;           \n"
		"                                   \n"
		//"uniform samplerExternalOES uTex;   \n"
		"uniform sampler2D uTex;   \n"
		"                                   \n"
		"varying vec4 vVaryingColor;        \n"
		"varying vec2 vTexCoord;            \n"
		"                                   \n"
		"void main()                        \n"
		"{                                  \n"
		"    gl_FragColor = vVaryingColor * texture2D(uTex, vTexCoord);\n"
		"}                                  \n";

static const char *vertex_shader_source_fs_quad =
		"uniform mat4 modelviewprojectionMatrix;\n"
		"                                   \n"
		"attribute vec4 in_position;        \n"
		"attribute vec2 in_TexCoord;        \n"
		"                                   \n"
		"varying vec2 vTexCoord;            \n"
		"                                   \n"
		"void main()                        \n"
		"{                                  \n"
		"    gl_Position = modelviewprojectionMatrix * in_position;\n"
		"    vTexCoord = in_TexCoord; \n"
		"}                            \n";

static const char *fragment_shader_source_fs_quad =
		//"#extension GL_OES_EGL_image_external : enable\n"
		"precision mediump float;           \n"
		"                                   \n"
		//"uniform samplerExternalOES uTex;   \n"
		"uniform sampler2D uTex;   \n"
		"                                   \n"
		"varying vec2 vTexCoord;            \n"
		"                                   \n"
		"void main()                        \n"
		"{                                  \n"
		"    gl_FragColor = texture2D(uTex, vTexCoord);\n"
		"}                                  \n";

#define GL(x)				{ x; glCheckError( #x, __FILE__, __LINE__ ); }
#define LOG_GL "[GL]"
static inline bool glCheckError(const char* msg, const char* file, int line)
{
	GLenum err = glGetError();

	if( err == GL_NO_ERROR )
		return false;

	const char* e = NULL;

	switch(err)
	{
		  case GL_INVALID_ENUM:          e = "invalid enum";      break;
		  case GL_INVALID_VALUE:         e = "invalid value";     break;
		  case GL_INVALID_OPERATION:     e = "invalid operation"; break;
		  case GL_STACK_OVERFLOW_KHR:        e = "stack overflow";    break;
		  case GL_STACK_UNDERFLOW_KHR:       e = "stack underflow";   break;
		  case GL_OUT_OF_MEMORY:         e = "out of memory";     break;
		#ifdef GL_TABLE_TOO_LARGE_EXT
		  case GL_TABLE_TOO_LARGE_EXT:   e = "table too large";   break;
		#endif
		#ifdef GL_TEXTURE_TOO_LARGE_EXT
		  case GL_TEXTURE_TOO_LARGE_EXT: e = "texture too large"; break;
		#endif
		  default:						 e = "unknown error";
	}

	LogError(LOG_GL "Error %i - '%s'\n", (uint)err, e);
	LogError(LOG_GL "   %s::%i\n", file, line );
	LogError(LOG_GL "   %s\n", msg );
	
	return true;
}

static bool init_cube(bool b_textured_cube)
{
	int ret = 0;

	if(b_textured_cube) {
		ret = create_program(vertex_shader_source_1img, fragment_shader_source_1img);
	} else {
		ret = create_program(vertex_shader_source, fragment_shader_source);
	}
	if (ret < 0)
		return false;

	gl.program = ret;

	glBindAttribLocation(gl.program, 0, "in_position");
	glBindAttribLocation(gl.program, 1, "in_normal");
	if(b_textured_cube) {
		glBindAttribLocation(gl.program, 2, "in_TexCoord");
	} else {
		glBindAttribLocation(gl.program, 2, "in_color");
	}

	ret = link_program(gl.program);
	if (ret)
		return false;

	glUseProgram(gl.program);

	gl.modelviewmatrix = glGetUniformLocation(gl.program, "modelviewMatrix");
	gl.modelviewprojectionmatrix = glGetUniformLocation(gl.program, "modelviewprojectionMatrix");
	gl.normalmatrix = glGetUniformLocation(gl.program, "normalMatrix");

	if(b_textured_cube) {
		gl.texture   = glGetUniformLocation(gl.program, "uTex");
	}

	gl.positionsoffset = 0;
	if(b_textured_cube) {
		gl.texcoordsoffset = sizeof(vVertices);
		gl.normalsoffset = sizeof(vVertices) + sizeof(vTexCoords);
	} else {
		gl.colorsoffset = sizeof(vVertices);
		gl.normalsoffset = sizeof(vVertices) + sizeof(vColors);
	}

	glBufferSubData(GL_ARRAY_BUFFER, gl.normalsoffset, sizeof(vNormals), &vNormals[0]);

	glGenBuffers(1, &gl.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, gl.vbo);
	if(b_textured_cube) {
		glBufferData(GL_ARRAY_BUFFER, sizeof(vVertices) + sizeof(vTexCoords) + sizeof(vNormals), 0, GL_STATIC_DRAW);
	} else {
		glBufferData(GL_ARRAY_BUFFER, sizeof(vVertices) + sizeof(vColors) + sizeof(vNormals), 0, GL_STATIC_DRAW);
	}

	glBufferSubData(GL_ARRAY_BUFFER, gl.positionsoffset, sizeof(vVertices), &vVertices[0]);

	if(b_textured_cube) {
		glBufferSubData(GL_ARRAY_BUFFER, gl.texcoordsoffset, sizeof(vTexCoords), &vTexCoords[0]);
	} else {
		glBufferSubData(GL_ARRAY_BUFFER, gl.colorsoffset, sizeof(vColors), &vColors[0]);
	}

	glBufferSubData(GL_ARRAY_BUFFER, gl.normalsoffset, sizeof(vNormals), &vNormals[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)(intptr_t)gl.positionsoffset);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)(intptr_t)gl.normalsoffset);
	glEnableVertexAttribArray(1);
	if(b_textured_cube) {
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)(intptr_t)gl.texcoordsoffset);
	} else {
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)(intptr_t)gl.colorsoffset);
	}
	glEnableVertexAttribArray(2);

	if(b_textured_cube) {
		ret = init_tex_rgba_ex(gbm->dev, &gl.egl, 512, 512, true, &gl.tex);
		if (ret) {
			printf("failed to initialize EGLImage texture\n");
			return false;
		}
	}

	return true;
}


static void draw_cube_smooth(unsigned i)
{
	SCOPED_TIMER("draw_cube_smooth");

	ESMatrix modelview;

	/* clear the color buffer */
	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	esMatrixLoadIdentity(&modelview);
	esTranslate(&modelview, 0.0f, 0.0f, -8.0f);
	esRotate(&modelview, 45.0f + (0.25f * i), 1.0f, 0.0f, 0.0f);
	esRotate(&modelview, 45.0f - (0.5f * i), 0.0f, 1.0f, 0.0f);
	esRotate(&modelview, 10.0f + (0.15f * i), 0.0f, 0.0f, 1.0f);

	ESMatrix projection;
	esMatrixLoadIdentity(&projection);
	esFrustum(&projection, -2.8f, +2.8f, -2.8f * gl.aspect, +2.8f * gl.aspect, 6.0f, 10.0f);

	ESMatrix modelviewprojection;
	esMatrixLoadIdentity(&modelviewprojection);
	esMatrixMultiply(&modelviewprojection, &modelview, &projection);

	float normal[9];
	normal[0] = modelview.m[0][0];
	normal[1] = modelview.m[0][1];
	normal[2] = modelview.m[0][2];
	normal[3] = modelview.m[1][0];
	normal[4] = modelview.m[1][1];
	normal[5] = modelview.m[1][2];
	normal[6] = modelview.m[2][0];
	normal[7] = modelview.m[2][1];
	normal[8] = modelview.m[2][2];

	glUniformMatrix4fv(gl.modelviewmatrix, 1, GL_FALSE, &modelview.m[0][0]);
	glUniformMatrix4fv(gl.modelviewprojectionmatrix, 1, GL_FALSE, &modelviewprojection.m[0][0]);
	glUniformMatrix3fv(gl.normalmatrix, 1, GL_FALSE, normal);

	if(b_textured_cube) {
		glUniform1i(gl.texture, 0); /* '0' refers to texture unit 0. */

		glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_EXTERNAL_OES, gl.tex[0]);
		glBindTexture(GL_TEXTURE_2D, dyn_texture.glTex->GetID());
	}

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);
	glDrawArrays(GL_TRIANGLE_STRIP, 8, 4);
	glDrawArrays(GL_TRIANGLE_STRIP, 12, 4);
	glDrawArrays(GL_TRIANGLE_STRIP, 16, 4);
	glDrawArrays(GL_TRIANGLE_STRIP, 20, 4);
}

static bool init_fs_quad()
{
	int ret = create_program(vertex_shader_source_fs_quad, fragment_shader_source_fs_quad);
	if (ret < 0)
		return false;

	gl.program = ret;

	glBindAttribLocation(gl.program, 0, "in_position");
	glBindAttribLocation(gl.program, 1, "in_TexCoord");

	ret = link_program(gl.program);
	if (ret)
		return false;

	glUseProgram(gl.program);

	gl.modelviewprojectionmatrix = glGetUniformLocation(gl.program, "modelviewprojectionMatrix");
	gl.texture   = glGetUniformLocation(gl.program, "uTex");

	gl.positionsoffset = 0;
	gl.texcoordsoffset = sizeof(vQuadVertices);

	glGenBuffers(1, &gl.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, gl.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vQuadVertices) + sizeof(vQuadTexCoords), 0, GL_STATIC_DRAW);

	glBufferSubData(GL_ARRAY_BUFFER, gl.positionsoffset, sizeof(vQuadVertices), &vQuadVertices[0]);
	glBufferSubData(GL_ARRAY_BUFFER, gl.texcoordsoffset, sizeof(vQuadTexCoords), &vQuadTexCoords[0]);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)(intptr_t)gl.positionsoffset);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)(intptr_t)gl.texcoordsoffset);
	glEnableVertexAttribArray(1);

	return true;
}


static void draw_fs_quad(unsigned i)
{
	SCOPED_TIMER("draw_fs_quad");

	ESMatrix modelview;

	/* clear the color buffer */
	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	esMatrixLoadIdentity(&modelview);
	esScale(&modelview, gbm->width, gbm->height, 1.0f);

	ESMatrix projection;
	esMatrixLoadIdentity(&projection);
	esOrtho(&projection, 0, gbm->width, gbm->height, 0, 0, 1);

	ESMatrix modelviewprojection;
	esMatrixLoadIdentity(&modelviewprojection);
	esMatrixMultiply(&modelviewprojection, &modelview, &projection);

	glUniformMatrix4fv(gl.modelviewprojectionmatrix, 1, GL_FALSE, &modelviewprojection.m[0][0]);

	glUniform1i(gl.texture, 0); /* '0' refers to texture unit 0. */
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, dyn_texture.glTex->GetID());

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}


// constructor
bool drmRenderer::init()
{
	// TODO: use mOptions they are initialized here already


	bool offscreen = false;
	bool atomic = false;
	const char *device = NULL;
	char mode_str[DRM_DISPLAY_MODE_LEN] = "";
	int connector_id = -1;
	unsigned int vrefresh = 0;

	uint32_t format = DRM_FORMAT_XRGB8888;
	uint64_t modifier = DRM_FORMAT_MOD_LINEAR;
	bool surfaceless = false;

	drm = init_drm_legacy(device, mode_str, connector_id, vrefresh, ~0);

	if (!drm) {
		printf("failed to initialize %s DRM\n",
		       offscreen ? "offscreen" :
		       atomic ? "atomic" : "legacy");
		return false;
	}

	gbm = init_gbm(drm->fd, drm->mode->hdisplay, drm->mode->vdisplay,
			format, modifier, surfaceless);
	if (!gbm) {
		printf("failed to initialize GBM\n");
		return false;
	}

	int samples = 1;
	int ret = init_egl(&gl.egl, gbm, samples);
	if (ret) {
		printf("failed to initialize EGL\n");
		return false;
	}

	//-----------------------------------------------------------------
	// During init, enable debug output
	glEnable(GL_DEBUG_OUTPUT_KHR);
	gl.egl.glDebugMessageCallbackKHR(MyMessageCallback, 0);
	//-----------------------------------------------------------------


	gl.aspect = (GLfloat)(gbm->height) / (GLfloat)(gbm->width);

	mOptions.width = gbm->width;
	mOptions.height = gbm->height;

//	if(0 != init_cube(b_textured_cube)) {
	if(!init_fs_quad()) {
		LogError(LOG_DRM "Failed to init_fs_quad()\n");
	}
	//gl.egl.draw = draw_cube_smooth;
	gl.egl.draw = draw_fs_quad;

	glViewport(0, 0, gbm->width, gbm->height);
	glEnable(GL_CULL_FACE);

	memset(&dyn_texture, 0, sizeof(dyn_texture));


	// Save the previous CRTC configuration
	mSavedCrtc = drmModeGetCrtc(drm->fd, drm->crtc_id);

	if(bUseDumbBuffers) {

		uint64_t b_has_dumb = false;
		if (drmGetCap(drm->fd, DRM_CAP_DUMB_BUFFER, &b_has_dumb) < 0 || !b_has_dumb) {
			LogError(LOG_DRM "drm device does not support dumb buffers\n");
			close(drm->fd);
			return false;
		}

		for(int i=0;i<mNumBuffers; ++i) {
			if(!create_drm_dumb_fb(drm->fd, drm->mode->hdisplay, drm->mode->vdisplay, &mFramebuffers[i])) {
				LogError(LOG_DRM "-- failed create_drm_dumb_fb");
				return false;
			}
			LogInfo(LOG_DRM " -- created framebuffer with ID %d\n", mFramebuffers[i].id);
		}

		// Perform the modeset
		ret = drmModeSetCrtc(drm->fd, drm->crtc_id, mFramebuffers[mCurBuffer].id, 0, 0, (uint32_t*)&(drm->connector_id), 1, drm->mode);
		if (ret < 0) {
			perror("drmModeSetCrtc");
		}
		mCurBuffer ^= 1;
	}
	else
	{
		legacy_run_setup(gbm, &gl.egl, &rfc);
	}

	fprintf(stderr, "2\n");

	return true;
}

drmRenderer::drmRenderer( const videoOptions& options ):videoOutput(options),render_init_cb_(nullptr), render_cb_(nullptr), uptr_(nullptr) {}

drmRenderer::~drmRenderer()
{
	// restore saved mode
	if(drm) {

		LogInfo(LOG_DRM " -- restoring old mode\n");
		drmModeSetCrtc(drm->fd, drm->crtc_id, mSavedCrtc->buffer_id, 
				mSavedCrtc->x, mSavedCrtc->y, (uint32_t*)&drm->connector_id, 1, &mSavedCrtc->mode);
		drmModeFreeCrtc(mSavedCrtc);

		if(bUseDumbBuffers) {
			for(int i=0;i<mNumBuffers; ++i) {
				free_drm_dumb_fb(drm->fd, &mFramebuffers[i]);
#if WITH_CUDA
				if(b_was_pinned[i]) {
					CUDA(cudaHostUnregister(mFramebuffers[i].data));
					b_was_pinned[i] = false;
				}
#endif
			}
		}

		close(drm->fd);
	}
}


// Create
drmRenderer* drmRenderer::Create( const videoOptions& options )
{
	drmRenderer* dr = new drmRenderer(options);
	if(!dr->init()) {
		delete dr;
		dr = nullptr;
	} else {
		dr->mStreaming = true;
	}
	return dr;
}

// Render
bool drmRenderer::Render( void* image, uint32_t width, uint32_t height, imageFormat format )
{
	const bool substreams_success = videoOutput::Render(image, width, height, format);

	SCOPED_TIMER("drmRenderer::Render");


	if(render_init_cb_ && !b_init_called) {
		(*render_init_cb_)(uptr_);
		b_init_called = true;
	}

	if(bUseDumbBuffers) {
		RenderDumb(image, width, height, format);
	} else {

		if(dyn_texture.glTex == 0) {
			extern const uint32_t raw_512x512_rgba[];
			uint8_t *src = (uint8_t *)raw_512x512_rgba;
			uint32_t glFormat = 0;
			if( format == IMAGE_RGB8 )
				glFormat = GL_RGB;
			else if( format == IMAGE_RGBA8 )
				glFormat = GL_RGBA;
			else {
				assert(0 && "Unsupported format");
			}
			dyn_texture.glTex = glTexture::Create(width, height, glFormat, nullptr);
		} else {

#if WITH_OPENCL
			{ SCOPED_TIMER("interopTex->Update");
#if 1
				cl_int status;
				void* mapped_ptr = clEnqueueMapBuffer(ocl_get_queue(), (cl_mem)image, CL_TRUE, CL_MAP_READ, 0, dyn_texture.glTex->GetSize(), 0, nullptr, nullptr, &status);
				CHECK_OPENCL_ERROR_NORET(status, "clEnqueueMapBuffer");

				dyn_texture.glTex->Update(GL_MAP_CPU, mapped_ptr);
				clEnqueueUnmapMemObject(ocl_get_queue(), (cl_mem)image, mapped_ptr, 0, nullptr, nullptr);
				CHECK_OPENCL_ERROR_NORET(status, "clEnqueueMapBuffer");
#endif
			}
#else
			printf("drmRenderer: CUDA Not implemented\n");
#endif
		}

		RenderGBM(image, width, height, format);
	}

	return substreams_success;
}

static void page_flip_handler(int fd, unsigned int frame, unsigned int sec, unsigned int usec, void *data)
{
	/* suppress 'unused parameter' warnings */
	(void)fd, (void)frame, (void)sec, (void)usec;

	int *waiting_for_flip = (int*)data;
	*waiting_for_flip = 0;
}


bool drmRenderer::RenderGBM( void* image, uint32_t width, uint32_t height, imageFormat format )
{
	SCOPED_TIMER("RenderGBM");

	const unsigned int frame = rfc.i;

	drmEventContext evctx = {
			.version = 2,
			.vblank_handler = nullptr,
			.page_flip_handler = page_flip_handler,
			.page_flip_handler2  = nullptr,
			.sequence_handler = nullptr,
	};

	struct egl* egl = &gl.egl;

	/* Start fps measuring on second frame, to remove the time spent
	 * compiling shader, etc, from the fps:
	 */
	if (rfc.i == 1) {
		rfc.start_time = rfc.report_time = timeNs();
	}

	if (!gbm->surface) {
		glBindFramebuffer(GL_FRAMEBUFFER, egl->fbs[frame % NUM_BUFFERS].fb);
	}

	egl->draw(rfc.i++);
	//draw_cube_smooth(rfc.i++);
	if(render_cb_) {
		SCOPED_TIMER("render_cb_");
		(*render_cb_)(uptr_, dyn_texture.glTex->GetID());
	}

	struct gbm_bo *next_bo;
	int waiting_for_flip = 1;

	if (gbm->surface) {
		{ SCOPED_TIMER("eglSwapBuffers");
		eglSwapBuffers(egl->display, egl->surface); }
		{ SCOPED_TIMER("gbm_surface_lock_front_buffer");
		next_bo = gbm_surface_lock_front_buffer(gbm->surface); }
	} else {
		SCOPED_TIMER("glFinish");
		glFinish();
		next_bo = gbm->bos[frame % NUM_BUFFERS];
	}
	rfc.fb = drm_fb_get_from_bo(next_bo);
	if (!rfc.fb) {
		fprintf(stderr, "Failed to get a new framebuffer BO\n");
		return -1;
	}

#if 0
	{ SCOPED_TIMER("drmModeSetCrtc");
	int ret = drmModeSetCrtc(drm->fd, drm->crtc_id, rfc.fb->fb_id, 0, 0, (uint32_t*)&(drm->connector_id), 1, drm->mode);
	if (ret < 0) {
		perror("drmModeSetCrtc");
	}
	}
#else
	// Here you could also update drm plane layers if you want hw composition

	int ret = drmModePageFlip(drm->fd, drm->crtc_id, rfc.fb->fb_id,
			DRM_MODE_PAGE_FLIP_EVENT, &waiting_for_flip);
	if (ret) {
		printf("failed to queue page flip: %s\n", strerror(errno));
		return -1;
	}

	fd_set fds;
	{
	SCOPED_TIMER("waiting_for_flip");
	while (waiting_for_flip) {
		FD_ZERO(&fds);
		// reading from stdin causes issues when running in backgroung
		// as we receive input which causes return and memory leaks
		// there is a fix for it on gitlab if needed, but for now 
		// I just disabled stdin fd polling
		//FD_SET(0, &fds);
		FD_SET(drm->fd, &fds);

		ret = select(drm->fd + 1, &fds, NULL, NULL, NULL);
		if (ret < 0) {
			printf("select err: %s\n", strerror(errno));
			return ret;
		} else if (ret == 0) {
			printf("select timeout!\n");
			return -1;
		} else if (FD_ISSET(0, &fds)) {
			printf("user interrupted!\n");
			exit(23);
			return 0;
		}
		drmHandleEvent(drm->fd, &evctx);
	}
	}
#endif

	uint64_t cur_time = timeNs();
	if (cur_time > (rfc.report_time + 2 * NSEC_PER_SEC)) {
		double elapsed_time = cur_time - rfc.start_time;
		double secs = elapsed_time / (double)NSEC_PER_SEC;
		unsigned frames = rfc.i - 1;  /* first frame ignored */
		printf("Rendered %u frames in %f sec (%f fps)\n",
				frames, secs, (double)frames/secs);
		rfc.report_time = cur_time;
	}

	/* release last buffer to render on again: */
	if (gbm->surface) {
		SCOPED_TIMER("gbm_surface_release_buffer");
		gbm_surface_release_buffer(gbm->surface, rfc.bo);
	}

	rfc.bo = next_bo;

	return 0;
}

bool drmRenderer::RenderDumb( void* image, uint32_t width, uint32_t height, imageFormat format )
{
	struct drm_dumb_fb* backbuffer = &mFramebuffers[mCurBuffer];

#if WITH_CUDA
	if(imageFormatChannels(format) == 3 && !b_was_pinned[mCurBuffer]) {
		SCOPED_TIMER("Pin");
		CUDA(cudaHostRegister(backbuffer->data, backbuffer->size, cudaHostRegisterMapped));
		CUDA(cudaHostGetDevicePointer(&cuda_dev_ptr[mCurBuffer], backbuffer->data, 0)); 
		b_was_pinned[mCurBuffer] = true;
	}

	{ SCOPED_TIMER("uchar3->uchar4");
		CUDA(cudaRGB8ToRGBA8((uchar3*)image, (uchar4*)cuda_dev_ptr[mCurBuffer], width, height, width, backbuffer->width, false));
	}

	if(imageFormatChannels(format) == 3 && !b_persistently_mapped) {
		SCOPED_TIMER("Unpin");
		CUDA(cudaHostUnregister(backbuffer->data));
		b_was_pinned[mCurBuffer] = false;
	}
#endif

#if WITH_OPENCL
	cl_int status;
	uint8_t* mapped_ptr = (uint8_t*)clEnqueueMapBuffer(ocl_get_queue(), (cl_mem)image, CL_TRUE, CL_MAP_READ, 0, width*height*imageFormatChannels(format), 0, nullptr, nullptr, &status);
	CHECK_OPENCL_ERROR_NORET(status, "clEnqueueMapBuffer");
	if(mapped_ptr) {
		if(imageFormatChannels(format) == 3)
		{ SCOPED_TIMER("uchar3 -> uchar4");
			for(int y = 0; y < std::min(height, backbuffer->height); ++y) {
				uint8_t* s = mapped_ptr + y*width*imageFormatChannels(format);
				uint8_t* d = backbuffer->data + y*backbuffer->stride;
				// last read will read one byte out of buffer, so width -1, we sacrifice one line for a pixel  :-)
				for(int x = 0; x< std::min(width, backbuffer->width) - 1; ++x) {
					*(uint32_t*)(d + 4*x) = *(uint32_t*)(s + 3*x);
				}
			}
		} else {
			SCOPED_TIMER("copy to surface");
			for(int y = 0; y < std::min(height, backbuffer->height); ++y) {
				uint8_t* s = mapped_ptr + y*width*imageFormatChannels(format);
				uint8_t* d = backbuffer->data + y*backbuffer->stride;
				memcpy(d, s, std::min((uint32_t)(width*imageFormatChannels(format)), backbuffer->stride));
			}
		}
		clEnqueueUnmapMemObject(ocl_get_queue(), (cl_mem)image, mapped_ptr, 0, nullptr, nullptr);
		CHECK_OPENCL_ERROR_NORET(status, "clEnqueueMapBuffer");
	}
#endif

	{ SCOPED_TIMER("drmModeSetCrtc");
	int ret = drmModeSetCrtc(drm->fd, drm->crtc_id, backbuffer->id, 0, 0, (uint32_t*)&(drm->connector_id), 1, drm->mode);
	if (ret < 0) {
		perror("drmModeSetCrtc");
	}
	}

	mCurBuffer ^= 1;

	//if(i==20)
	//	mStreaming = false;

	return 0;
}

