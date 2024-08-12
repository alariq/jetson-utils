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

	DearImguiDisplay(const videoOptions& options) :glDisplay(options), render_init_cb_(0), render_cb_(0), uptr_(0), b_render_init_called(false),
	drag_finished_cb_(0), drag_cb_uptr_(0), key_cb_(0), key_cb_uptr_(0) {
		ImgPosAbs[0] = ImgPosAbs[1] = 0;
	}
	~DearImguiDisplay();
	static DearImguiDisplay* Create( const videoOptions& options );
	virtual bool Render(void* image, uint32_t width, uint32_t height, imageFormat format) override;
	virtual void SetStatus(const char* str) override;
	virtual void SetRenderCallback(videoOutput::RenderCallback_t cb_init, videoOutput::RenderRunCallback_t cb_run, void* uptr) {
		render_init_cb_ = cb_init;
		render_cb_ = cb_run;
		uptr_ = uptr;
	}

	virtual void SetDragFinishedCallback(glDisplay::DragFinishedCallback_t drag_finished_cb, void* user_ptr) override {
		drag_finished_cb_ = drag_finished_cb;
		drag_cb_uptr_ = user_ptr;
	}

	virtual void SetKeyboardCallback(videoOutput::KeyboardCallback_t key_cb, void* uptr) {
		key_cb_ = key_cb;
		key_cb_uptr_ = uptr;
	}

	static void keyCallback(struct GLFWwindow* window, int key, int scancode, int action, int mods);

	virtual struct __GLXcontextRec* GetGLXContext() const override;
	virtual struct _XDisplay* GetX11Display() const override;

	void* GetEGLContext() const;
	void* GetEGLDisplay() const;


private:
	bool Init();
	struct GLFWwindow* glfwWindow_;
	videoOutput::RenderCallback_t render_init_cb_;
	videoOutput::RenderRunCallback_t render_cb_;
	void* uptr_;
     bool b_render_init_called;
	int ImgPosAbs[2];

	glDisplay::DragFinishedCallback_t drag_finished_cb_;
	void* drag_cb_uptr_;
	int mLastBBox[4];

	videoOutput::KeyboardCallback_t key_cb_;
	void* key_cb_uptr_;

	bool mImmersionMode = false;


	glTexture* last_rendered_image_ = nullptr;
};
