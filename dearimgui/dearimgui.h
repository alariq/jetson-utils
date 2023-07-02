#pragma once

#include "glDisplay.h"
#include "imageFormat.h"

#include <stdint.h>

class DearImguiDisplay: public glDisplay {

public:
	DearImguiDisplay(const videoOptions& options):glDisplay(options) {}
	~DearImguiDisplay();
	static DearImguiDisplay* Create( const videoOptions& options );
	virtual bool Render(void* image, uint32_t width, uint32_t height, imageFormat format) override;
	virtual void SetStatus(const char* str) override;

private:
	bool Init();
	struct GLFWwindow* glfwWindow_;
};