#pragma once

#include "glDisplay.h"
#include "videoOutput.h"
#include "imageFormat.h"

#include <stdint.h>

class DearImguiDisplay: public glDisplay {

public:

	/**
	 * Return the interface type (DearImguiDisplay::Type)
	 */
	virtual inline uint32_t GetType() const		{ return Type; }

	/**
	 * Unique type identifier of glDisplay class.
	 */
	static const uint32_t Type = (1 << 6);

	virtual void SetMaximized(bool maximized);
	virtual bool IsMaximized();
	virtual void SetFullscreen(bool fullscreen);
	virtual bool IsFullscreen();

	DearImguiDisplay(const videoOptions& options) :glDisplay(options), render_cb_(0), uptr_(0) {
		ImgPosAbs[0] = ImgPosAbs[1] = 0;
	}
	~DearImguiDisplay();
	static DearImguiDisplay* Create( const videoOptions& options );
	virtual bool Render(void* image, uint32_t width, uint32_t height, imageFormat format) override;
	virtual void SetStatus(const char* str) override;
	virtual void SetRenderCallback(videoOutput::RenderCallback_t cb, void* uptr) override {
		render_cb_ = cb;
		uptr_ = uptr;
	}

	virtual void SetDragFinishedCallback(glDisplay::DragFinishedCallback_t drag_finished_cb, void* user_ptr) override {
		drag_finished_cb_ = drag_finished_cb;
		drag_cb_uptr_ = user_ptr;
	}

private:
	bool Init();
	struct GLFWwindow* glfwWindow_;
	videoOutput::RenderCallback_t render_cb_;
	void* uptr_;
	int ImgPosAbs[2];

	glDisplay::DragFinishedCallback_t drag_finished_cb_;
	void* drag_cb_uptr_;
	int mLastBBox[4];
};
