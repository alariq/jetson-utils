#pragma once

#include "glDisplay.h"
#include "videoOutput.h"
#include "imageFormat.h"

#include <stdint.h>

class DearImguiDisplay: public glDisplay {

public:
	DearImguiDisplay(const videoOptions& options):glDisplay(options),render_cb_(0), uptr_(0) {}
	~DearImguiDisplay();
	static DearImguiDisplay* Create( const videoOptions& options );
	virtual bool Render(void* image, uint32_t width, uint32_t height, imageFormat format) override;
	virtual void SetStatus(const char* str) override;
	virtual void SetRenderCallback(videoOutput::RenderCallback_t cb, void* uptr) override {
		render_cb_ = cb;
		uptr_ = uptr;
	}

private:
	bool Init();
	struct GLFWwindow* glfwWindow_;
	videoOutput::RenderCallback_t render_cb_;
	void* uptr_;
};