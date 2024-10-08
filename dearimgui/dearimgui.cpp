#include "dearimgui.h"

#include "glDisplay.h"
#include "glTexture.h"
#include "imageFormat.h"
#include "timespec.h"

//#include "imgui.h"
#define IMGUI_IMPL_OPENGL_DEBUG
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"

#include <stdint.h>

#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_GLX
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_EGL
//#define GLFW_NATIVE_INCLUDE_NONE
#include <GLFW/glfw3native.h>

#include <X11/Xlib.h> // Display
#include <GL/glx.h> // Context
//#include <EGL/egl.h> // Context/Display


extern uint32_t glAddDisplay(glDisplay* display);

static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

DearImguiDisplay* DearImguiDisplay::Create( const videoOptions& options )
{
	DearImguiDisplay* d = new DearImguiDisplay(options);
	d->glfwWindow_ = nullptr;

	if(d->Init()) {
		return d;
	} else {
		delete d;
		return 0;
	}
}

bool DearImguiDisplay::Init()
{
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		return false;

	// GL 3.0 + GLSL 130
	const char* glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

	// Create window with graphics context
	int defaultWidth = 1280;
	int defaultHeight = 720;
	if(mOptions.width == 0) {
		mOptions.width = defaultWidth;
	}

	if(mOptions.height == 0) {
		mOptions.height = defaultHeight;
	}

	GLFWmonitor *monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
	//glfwWindowHint(GLFW_DOUBLEBUFFER, 0);
	LogInfo("dearimgui: Refresh rate: %d\n", mode->refreshRate);

	//GLFWwindow* window = glfwCreateWindow(mOptions.width, mOptions.height, "Dear ImGui GLFW+OpenGL3", nullptr, nullptr);
	GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Dear ImGui GLFW+OpenGL3", nullptr, nullptr);
	if (window == nullptr)
		return false;
	glfwWindow_ = window;
	glfwMakeContextCurrent(window);
	glfwSwapInterval(mOptions.swapInterval);

	if(!glfwExtensionSupported("GLX_EXT_swap_control_tear")) {
		LogInfo("dearimgui GLX_EXT_swap_control_tear NOT supported");
	}

	glfwSetKeyCallback(glfwWindow_, keyCallback);
	glfwSetWindowUserPointer(glfwWindow_, this);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
     ImPlot::CreateContext();
     ImGuiIO& io = ImGui::GetIO(); (void)io;

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	//ImGui::StyleColorsDark();
	ImGui::StyleColorsLight();
	ImPlot::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

#if !defined(USE_OPENGL_ES2)
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		LogError(LOG_GL "GLEW Error: %s\n", glewGetErrorString(err));
		return false;
	}
#endif

	mScreenWidth  = mOptions.width;
	mScreenHeight = mOptions.height;

	mViewport[0] = 0; 
	mViewport[1] = 0; 
	mViewport[2] = mOptions.width; 
	mViewport[3] = mOptions.height;

	mStreaming = true;

	mID = glAddDisplay(this);

	LogInfo(LOG_GL "ImGuiDisplay -- display device initialized (%ux%u)\n", GetWidth(), GetHeight());

	return true;
}

GLXContext DearImguiDisplay::GetGLXContext() const {
	return glfwGetGLXContext(glfwWindow_);
}

Display* DearImguiDisplay::GetX11Display() const {
	return glfwGetX11Display();
}

EGLContext DearImguiDisplay::GetEGLContext() const {
	return glfwGetEGLContext(glfwWindow_);
}

EGLDisplay DearImguiDisplay::GetEGLDisplay() const {
	return glfwGetEGLDisplay();
}

void DearImguiDisplay::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	DearImguiDisplay* me = (DearImguiDisplay*)glfwGetWindowUserPointer(window);
	if (key == GLFW_KEY_F && action == GLFW_PRESS) {
		me->mImmersionMode = !me->mImmersionMode;
	}

	if(me->key_cb_) {
		me->key_cb_(me->key_cb_uptr_, key, scancode, action, mods);
	}
}

bool DearImguiDisplay::Render(void* image, uint32_t width, uint32_t height, imageFormat format)
{
	SCOPED_TIMER("DearImguiDisplay::Render");

	ImGuiIO& io = ImGui::GetIO(); (void)io;

	if(render_init_cb_ && !b_render_init_called) {
		render_init_cb_(uptr_);
		b_render_init_called = true;
	}

	glfwPollEvents();

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	static bool show_demo_window = false;
	static bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	if(glfwWindowShouldClose(glfwWindow_))
	{
		mStreaming = false;
		return true;
	}

	ImVec2 mousePositionAbsolute = ImGui::GetMousePos();
	mMousePos[0] = (int)mousePositionAbsolute.x;
	mMousePos[1] = (int)mousePositionAbsolute.y;

	bool RMB_Down = ImGui::IsMouseDown(ImGuiMouseButton_Right);
	bool LMB_Down = ImGui::IsMouseDown(ImGuiMouseButton_Left);
	bool MMB_Down = ImGui::IsMouseDown(ImGuiMouseButton_Middle);

	bool mouseButtonsPrev[sizeof(mMouseButtons)/sizeof(mMouseButtons[0])];
	memcpy(mouseButtonsPrev, mMouseButtons, sizeof(mMouseButtons));
	if(!mMouseButtons[MOUSE_RIGHT] && RMB_Down) {
		if(mDragMode != DragDisabled) {
			mMouseDragOrigin[0] = mMousePos[0];
			mMouseDragOrigin[1] = mMousePos[1];
			LogInfo(LOG_GL "Started Drag with x: %d y: %d\n", mMouseDragOrigin[0], mMouseDragOrigin[1]);
		}
	}

	if(RMB_Down && mMouseDragOrigin[0] >=0 && mMouseDragOrigin[1] >=0) {
		mMouseDrag[0] = mMousePos[0];
		mMouseDrag[1] = mMousePos[1];
		LogDebug(LOG_GL "Current Drag pos x: %d y: %d W: %d H: %d\n", mMouseDrag[0], mMouseDrag[1], mOptions.width, mOptions.height);
	}

	const bool bDragFinished = mMouseButtons[MOUSE_RIGHT] && !RMB_Down;
	if(bDragFinished) {
		mMouseDragOrigin[0] = -1;
		mMouseDragOrigin[1] = -1;
		mMouseDrag[0] = -1;
		mMouseDrag[1] = -1;
	}

	mMouseButtons[MOUSE_RIGHT] = RMB_Down;
	mMouseButtons[MOUSE_LEFT] = LMB_Down;
	mMouseButtons[MOUSE_MIDDLE] = MMB_Down;

	if (show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
	if(0)
	{
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
		ImGui::Checkbox("Another Window", &show_another_window);

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		ImGui::End();
	}

	// 3. Show another simple window.
	if (show_another_window)
	{
		ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		ImGui::Text("Hello from another window!");
		if (ImGui::Button("Close Me"))
			show_another_window = false;
		ImGui::End();
	}

	//glDisplay::Render(image, width, height, format);
	ImVec2 imgWinSize = ImVec2(0,0);
	bool display_success = true;
	glTexture* tex = nullptr;
	if (image || last_rendered_image_) {

		// determine input format
		if (imageFormatIsRGB(format))
		{
			// resize the window once to match the feed, but let the user resize/maximize
			// only resize again if the window is then smaller than the feed
			//if (!mResizedToFeed || ((GetWidth() < width || GetHeight() < height) && (width < mScreenWidth && height < mScreenHeight)))
			//{
			//	SetSize(width, height);
			//	mResizedToFeed = true;
			//}

			// obtain the OpenGL texture to use
			//glTexture* interopTex = allocTexture(width, height, format);
			//if(!interopTex) {
			//}

			tex = last_rendered_image_;
			if(image) {
				tex = PrepareImage(image, width, height, format, 0, 0, true);
				last_rendered_image_ = tex;
			}

			if (!mImmersionMode) {
				ImGui::Begin("#main_image", nullptr, ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoTitleBar);
				//ImGui::Text("pointer = %p", tex);
				ImGui::Image((void*)(intptr_t)tex->GetID(), ImVec2(tex->GetWidth(), tex->GetHeight()));
				bool isHovered = ImGui::IsItemHovered();
				bool isFocused = ImGui::IsItemFocused();
				ImVec2 winPos = ImGui::GetItemRectMin();
				imgWinSize = ImGui::GetItemRectSize();
				ImgPosAbs[0] = (int)winPos.x;
				ImgPosAbs[1] = (int)winPos.y;
				ImGui::End();
			} else {
				// to not hide menu bar, more correct will be to use GetWindowSize() while inside Menu bar drawing,
				// because it wll account for style.DisplaySafeAreaPadding (may be useful for TV sets), but menu drawing
				// is not happening here, probably need to rework this part, so main image is also happens on a "client" side
				// by just calling some function like get_image_id()
				float fh = ImGui::GetFrameHeight();

				ImVec2 pos = ImGui::GetMainViewport()->Pos;
				ImVec2 size = ImGui::GetMainViewport()->Size;
				ImVec2 minp = ImVec2(pos.x, pos.y + fh);
				ImVec2 maxp = ImVec2(pos.x + size.x, pos.y + size.y);
				ImGui::GetForegroundDrawList()->AddImage((void*)(intptr_t)tex->GetID(), minp, maxp);

				imgWinSize = ImVec2(maxp.x - minp.x, maxp.y - minp.y);
				ImgPosAbs[0] = (int)minp.x;
				ImgPosAbs[1] = (int)minp.y;
			}

		} else {
			LogError(LOG_GL "glDisplay::Render() -- unsupported image format (%s)\n", imageFormatToStr(format));
			LogError(LOG_GL "                       supported formats are:\n");
			LogError(LOG_GL "                           * rgb8\n");
			LogError(LOG_GL "                           * rgba8\n");
			LogError(LOG_GL "                           * rgb32\n");
			LogError(LOG_GL "                           * rgba32\n");
			display_success = false;
		}
	}

	if(render_cb_ /*&& !mImmersionMode*/) {
		render_cb_(uptr_, tex ? tex->GetID() : 0);
	}


	{
	SCOPED_TIMER("ImGui::Render");
	ImGui::Render();
	}
	int display_w, display_h;
	glfwGetFramebufferSize(glfwWindow_, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
	glClear(GL_COLOR_BUFFER_BIT);


	// render sub-streams
	const bool substreams_success = videoOutput::Render(image, width, height, format);

	{
	SCOPED_TIMER("ImGui_ImplOpenGL3_RenderDrawData");
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	// update viewport
	mOptions.width = display_w;
	mOptions.height = display_h;
	mViewport[2] = mOptions.width;
	mViewport[3] = mOptions.height;
	SetViewport(0, 0, display_w, display_h);

	activateViewport();
	if( (mDragMode == DragSelect || mDragMode == DragCreate) && IsDragging(mDragMode) )
	{
		int x, y;
		int width, height;

		if( GetDragRect(&x, &y, &width, &height) ) {
			RenderOutline(x, y, width, height, 1, 1, 1);
			mLastBBox[0] = x;
			mLastBBox[1] = y;
			mLastBBox[2] = width;
			mLastBBox[3] = height;
		}
	}
	
	{
	SCOPED_TIMER("glfwSwapBuffers");
	glfwSwapBuffers(glfwWindow_);
	}

	if(bDragFinished && drag_finished_cb_) {
		int bbX0 = mLastBBox[0];
		int bbY0 = mLastBBox[1];
		int bbX1 = mLastBBox[0] + mLastBBox[2];
		int bbY1 = mLastBBox[1] + mLastBBox[3];
		
		int wX0 = ImgPosAbs[0];
		int wY0 = ImgPosAbs[1];
		int wX1 = wX0 + imgWinSize.x;
		int wY1 = wY0 + imgWinSize.y;

		// only call it if bb overlaps window
		if (bbX0 < wX1 && bbY0 < wY1 && bbX1 > wX0 && bbY1 > wY0) {
			bbX0 = std::max(bbX0, wX0);
			bbY0 = std::max(bbY0, wY0);
			bbX1 = std::min(bbX1, wX1);
			bbY1 = std::min(bbY1, wY1);
			// relative to the image widow
			// and in relative coordinates because aspect/size 
			// can be different on remote
			float x = (bbX0 - wX0) / (float)imgWinSize.x;
			float y = (bbY0 - wY0) / (float)imgWinSize.y;
			float w = (bbX1 - bbX0) / (float)imgWinSize.x;
			float h = (bbY1 - bbY0) / (float)imgWinSize.y;
			drag_finished_cb_(x, y, w, h, drag_cb_uptr_);
		}	
	}

	return display_success && substreams_success;
}

void DearImguiDisplay::SetStatus(const char* str) {
	if(glfwWindow_) {
		glfwSetWindowTitle(glfwWindow_, str);
	}
}

void DearImguiDisplay::SetMaximized( bool maximized ) {
	if(maximized) {
		glfwMaximizeWindow(glfwWindow_);
	} else {
		glfwRestoreWindow(glfwWindow_);
	}
}

bool DearImguiDisplay::IsMaximized() {
	return glfwGetWindowAttrib(glfwWindow_, GLFW_MAXIMIZED);
}
void DearImguiDisplay::SetFullscreen( bool fullscreen ) {
	GLFWmonitor *monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode *mode = glfwGetVideoMode(monitor);
	glfwSetWindowMonitor(glfwWindow_, fullscreen ? monitor : nullptr, 0, 0, mode->width, mode->height, mode->refreshRate);
}

bool DearImguiDisplay::IsFullscreen() {
	return nullptr != glfwGetWindowMonitor(glfwWindow_);
}

DearImguiDisplay::~DearImguiDisplay() {

	ReleaseRenderResources();

    ImPlot::DestroyContext();

    ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(glfwWindow_);
	glfwTerminate();
	
	fprintf(stderr, "~dearimgui display\n");
}

