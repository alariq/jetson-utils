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

#include "videoSource.h"
#include "videoOutput.h"

#include "glDisplay.h"
#include "drmRenderer.h"
#if WITH_IMGUI
#include "dearimgui.h"
#endif

#include "logging.h"
#include "commandLine.h"

#if WITH_OPENCL
#include "oclColorspace.h"
#include "ocl_utils.h"
#endif

#include "timespec.h"

#if WITH_IMGUI
// should be visible for .cpp as well, so shoudl be system wide
//#define IMGUI_IMPL_OPENGL_ES2
#define IMGUI_IMPL_OPENGL_DEBUG
#include "../../imgui/backends/imgui_impl_opengl3.h"
#include "../../imgui/backends/imgui_impl_glfw.h"
#include "../../implot/implot.h"
#endif

#include <signal.h>
#include <unistd.h> // getlogin_r


bool signal_recieved = false;

void sig_handler(int signo)
{
	if( signo == SIGINT )
	{
		LogInfo("received SIGINT\n");
		signal_recieved = true;
	}
}

int usage()
{
	printf("usage: video-viewer [--help] input_URI [output_URI]\n\n");
	printf("View/output a video or image stream.\n");
	printf("See below for additional arguments that may not be shown above.\n\n");
	printf("positional arguments:\n");
	printf("    input_URI       resource URI of input stream  (see videoSource below)\n");
	printf("    output_URI      resource URI of output stream (see videoOutput below)\n\n");

	printf("%s", videoSource::Usage());
	printf("%s", videoOutput::Usage());
	printf("%s", Log::Usage());

	return 0;
}

void OnDragFinished(float rx, float ry, float rw, float rh, void* user_ptr) {
	printf("bbox X: %.2f Y: %.2f W: %.2f H: %.2f\n", rx, ry, rw, rh);
}

ImFont* big_font = nullptr; 

void render_init(void* uptr) {

#if WITH_IMGUI
	videoOutput* output = (videoOutput*)uptr;
	if(output->GetType() != DearImguiDisplay::Type) {

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
		//ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init(nullptr);

		io.DisplaySize.x = output->GetWidth();             // set the current display width
		io.DisplaySize.y = output->GetHeight();             // set the current display height here
												  //
	}

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.Fonts->AddFontDefault();
	ImFontConfig config;
	config.SizePixels = 48;
	big_font = io.Fonts->AddFontFromFileTTF("Roboto-Medium.ttf", 48);//, &config);
	assert(big_font);
#endif
}

void render_run(void* uptr, uint32_t tex_id) {

	SCOPED_TIMER("imgui render");

	videoOutput* output = (videoOutput*)uptr;

	if(output->GetType() != DearImguiDisplay::Type) {
		ImGui_ImplOpenGL3_NewFrame();
		//ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	ImGuiIO& io = ImGui::GetIO(); (void)io;

	int img_width = output->GetWidth();
	int img_height = output->GetHeight();

	static bool show_demo_window = false;

	static bool show_another_window = false;
	float clear_color[4] = { 100,100,100,255};
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

	{
		ImVec4 box = ImVec4(10, 10, 200, 200);
		const ImVec2 vMin = ImVec2(0,0);//ImGui::GetWindowContentRegionMin();
		const ImVec2 vMax = io.DisplaySize;//ImGui::GetWindowContentRegionMax();

		const float kw = 0 + 1*(vMax.x - vMin.x)/img_width;
		const float kh = 0 + 1*(vMax.y - vMin.y)/img_height;

		ImVec2 pmin(box.x*kw, box.y*kh);
		ImVec2 pmax(box.z*kw, box.w*kh);

		ImGui::GetForegroundDrawList()->AddRect( pmin, pmax, IM_COL32( 100, 255, 100, 255 ), 8, 0, 4);

		if(big_font) {
			ImGui::PushFont(big_font);
			ImGui::GetForegroundDrawList()->AddText(pmin, IM_COL32( 255, 255, 0, 255 ), "Simple text");
			ImGui::PopFont();
		}
	}

	if(output->GetType() != DearImguiDisplay::Type) {
		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
}

int main( int argc, char** argv )
{
	/*
	 * parse command line
	 */
	commandLine cmdLine(argc, argv);

	if( cmdLine.GetFlag("help") )
		return usage();


	/*
	 * attach signal handler
	 */	
	if( signal(SIGINT, sig_handler) == SIG_ERR )
		LogError("can't catch SIGINT\n");


	/*
	 * create input video stream
	 */
	videoSource* input = videoSource::Create(cmdLine, ARG_POSITION(0));

	if( !input )
	{
		LogError("video-viewer:  failed to create input stream\n");
		return 0;
	}
#if WITH_OPENCL
	oclInitParams oclinitparams;
	memset(&oclinitparams, 0, sizeof(oclinitparams));

	oclinitparams.device_type_ = CL_DEVICE_TYPE_GPU;
#endif
	/*
	 * create output video stream
	 */
	videoOutput* output = videoOutput::Create(cmdLine, ARG_POSITION(1));
#if 0
	if(output->GetType() == DearImguiDisplay::Type || output->GetType() == glDisplay::Type) {
			glDisplay* o = ((DearImguiDisplay*)output);
			o->SetDragMode(glDisplay::DragSelect);
			o->SetDragFinishedCallback(OnDragFinished, nullptr);
			// TODO: looks like OpenCL context should be created in some other place, because even if we use
			// file:// or rtsp:// out, glDisplay output is still created, so maybe need to enumerate all videoOutput(s) to find out if we need OpenCL interop
#if WITH_OPENCL
#if defined(__x86_64__) || defined(__amd64__)
			oclinitparams.display_ = o->GetX11Display();
			oclinitparams.glcontext_ = o->GetGLXContext();
			oclinitparams.b_enable_interop = true;
#else
			oclinitparams.b_enable_import_memory_arm = true;
#endif
#endif
	}
	
#endif // 0
	
	if( !output )
	{
		LogError("video-viewer:  failed to create output stream\n");
		return 0;
	}

	output->SetRenderCallback(&render_init, &render_run, output);


#if WITH_OPENCL
	char user_name[256];
	if(getlogin_r(user_name, sizeof(user_name))) {
		strcpy(user_name, "cat");
	}

	char path2kernels[1024];
	// TODO: add this path in cmake scripts as -DKERNEL_PATH = something derived from INSTALL DIR ?
	sprintf(path2kernels, "/home/%s/external-nocuda/include/jetson-utils", &user_name[0]);

	if(!oclInit(&oclinitparams, path2kernels)) {
		LogError("Failed to init OpenCL\n");
	}
#endif


	/*
	 * capture/display loop
	 */
	uint32_t numFrames = 0;

	while( !signal_recieved )
	{
		void* image = NULL;
		imageFormat fmt = IMAGE_RGB8;
		uint64_t timeout = videoSource::DEFAULT_TIMEOUT;
		int status = 0;
		
		timespec ts1 = timestamp();
		if( !input->Capture(&image, fmt, timeout, &status) )
		{
			if( status == videoSource::TIMEOUT )
				continue;
			
			break; // EOS
		}
		timespec ts2 = timestamp();

		
		if( output != NULL )
		{

			SCOPED_TIMER("Render");

			output->Render(image, input->GetWidth(), input->GetHeight(), fmt);

			// update status bar
			char str[256];
			sprintf(str, "Video Viewer (%ux%u) | %.1f FPS", input->GetWidth(), input->GetHeight(), output->GetFrameRate());
			output->SetStatus(str);	

			// check if the user quit
			if( !output->IsStreaming() )
				break;
		}

		timespec ts3 = timestamp();

		float cap_ms = timeFloat(timeDiff(ts1, ts2));
		float rnd_ms = timeFloat(timeDiff(ts2, ts3));

		if( numFrames % 25 == 0 || numFrames < 15 )
			LogVerbose("video-viewer:  captured %u frames (%ux%u) Capture: %.2fms Render: %.2fms\n", numFrames, input->GetWidth(), input->GetHeight(), cap_ms, rnd_ms);
		
		numFrames++;
	}


	/*
	 * destroy resources
	 */
	printf("video-viewer:  shutting down...\n");
	
	SAFE_DELETE(input);
	SAFE_DELETE(output);

	printf("video-viewer:  shutdown complete\n");
}

