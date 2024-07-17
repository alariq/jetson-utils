#pragma once
#include "videoOutput.h"

 /*
 * @see videoOutput
 * @ingroup drm 
 */
class drmRenderer : public videoOutput
{
public:
	/**
	 * Create an drmRenderer instance from the provided video options.
	 */
	static drmRenderer* Create( const videoOptions& options );

	/**
	 * Destructor
	 */
	virtual ~drmRenderer();

	/**
	 * Save the next frame.
	 * @see videoOutput::Render()
	 */
	template<typename T> bool Render( T* image, uint32_t width, uint32_t height )		{ return Render((void**)image, width, height, imageFormatFromType<T>()); }
	
	/**
	 * Save the next frame.
	 * @see videoOutput::Render()
	 */
	virtual bool Render( void* image, uint32_t width, uint32_t height, imageFormat format );

	/**
	 * Return the interface type (drmRenderer::Type)
	 */
	virtual inline uint32_t GetType() const		{ return Type; }

	/**
	 * Unique type identifier of drmRenderer class.
	 */
	static const uint32_t Type = (1 << 7);

	virtual void SetRenderCallback(videoOutput::RenderCallback_t cb_init, videoOutput::RenderCallback_t cb_run, void* uptr) override {
		render_init_cb_ = cb_init;
		render_cb_ = cb_run;
		uptr_ = uptr;
	}

protected:

	bool RenderDumb( void* image, uint32_t width, uint32_t height, imageFormat format );
	bool RenderGBM( void* image, uint32_t width, uint32_t height, imageFormat format );

	drmRenderer( const videoOptions& options );
	bool init();

	videoOutput::RenderCallback_t render_init_cb_;
	videoOutput::RenderCallback_t render_cb_;
	void* uptr_;
	bool b_init_called = false;
};

