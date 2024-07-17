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

#include "oclColorspace.h"

//#include "cudaRGB.h"
#include "oclYUYV.h"
#include "oclYV12.h"
#include "oclNV12.h"
//#include "cudaBayer.h"
//#include "cudaGrayscale.h"

#include "logging.h"
#include "ocl_utils.h"
#include "ocl_program.h"
#include "CL/cl.h"

#include <assert.h>

static ocl_program I420_to_RGB_prg;
static ocl_program YV12_to_RGB_prg;

static ocl_program YUYV_to_RGB_prg;
static ocl_program YVYU_to_RGB_prg;
static ocl_program UYVY_to_RGB_prg;

static ocl_program NV12_to_RGB_prg;


static ocl_program RGB_to_I420_prg;


cl_int oclInit(oclInitParams* oclinitparams, const char* kernel_root_dir) {

    //TODO: only for Debug
    cl_command_queue_properties props = CL_QUEUE_PROFILING_ENABLE;
    if(!ocl_init(oclinitparams, props))
    {
        LogError("Failed to init OpenCL\n");
        return false;
    }

	bool b_success = true;

	struct __ {
		ocl_program* p;
		const char* prog;
		const char* defines;
		const char* kname;
	};

	__ config[] = {
		// To RGB
		// ------------------------------------------------------------
		{&YUYV_to_RGB_prg, "YUYV_to_RGB.cl", "-DFMT_YUYV", "YUYVToRGB"},
		{&YVYU_to_RGB_prg, "YUYV_to_RGB.cl", "-DFMT_YVYU", "YUYVToRGB"},
		{&UYVY_to_RGB_prg, "YUYV_to_RGB.cl", "-DFMT_UYVY", "YUYVToRGB"},

		{&I420_to_RGB_prg, "I420_to_RGB.cl", "-DIS_420=1 -DRGB_TO_YV12=0", "I420ToRGB"},
		{&YV12_to_RGB_prg, "I420_to_RGB.cl", "-DIS_420=0 -DRGB_TO_YV12=0", "I420ToRGB"},

		{&NV12_to_RGB_prg, "NV12_to_RGB.cl", "", "NV12ToRGB"},

		// From RGB
		// ------------------------------------------------------------
		{&RGB_to_I420_prg, "I420_to_RGB.cl", "-DIS_420=0 -DSRC_TYPE=uchar -DRGB_TO_YV12=1", "RGBToYV12"},
	};

	char prog[1024];
	for(int i=0;i<sizeof(config)/sizeof(config[0]); ++i) {
		sprintf(prog, "%s/%s", kernel_root_dir, config[i].prog);

		LogInfo("Loading %s\n", prog); 
		if(!config[i].p->load(prog, config[i].defines, ocl_get_context(), ocl_get_devices() + ocl_get_device_id(), 1)) {
			LogError("Failed to load program file: %s\n", config[i].prog);
			return false;
		}
		// TODO: maybe add 3 & 4 channel variant + uint8/fp32
		b_success = b_success && config[i].p->create_kernel(config[i].kname); 
	}

	return b_success;

}


// oclConvertColor
cl_int oclConvertColor(cl_mem input, imageFormat inputFormat,
					     cl_mem output, imageFormat outputFormat,
					     size_t width, size_t height,
						const float pixel_range_min, const float pixel_range_max)
{
	if( inputFormat == IMAGE_NV12 )
	{
		if( outputFormat == IMAGE_RGB8 )
			return OCL(oclNV12ToRGB8(NV12_to_RGB_prg, input, output, width, height));
		else if( outputFormat == IMAGE_RGB32F )
			return OCL(oclNV12ToRGB32F(NV12_to_RGB_prg, input, output, width, height));
		else if( outputFormat == IMAGE_RGBA8 )
			return OCL(oclNV12ToRGBA8(NV12_to_RGB_prg, input, output, width, height));
		else if( outputFormat == IMAGE_RGBA32F )
			return OCL(oclNV12ToRGBA32F(NV12_to_RGB_prg, input, output, width, height));
	}
	else if( inputFormat == IMAGE_I420 )
	{
		if( outputFormat == IMAGE_RGB8 )
			return OCL(oclI420ToRGB8(I420_to_RGB_prg, input, output, width, height));
		else if( outputFormat == IMAGE_RGB32F )
			return OCL(oclI420ToRGB32F(I420_to_RGB_prg, input, output, width, height));
		else if( outputFormat == IMAGE_RGBA8 )
			return OCL(oclI420ToRGBA8(I420_to_RGB_prg, input, output, width, height));
		else if( outputFormat == IMAGE_RGBA32F )
			return OCL(oclI420ToRGBA32F(I420_to_RGB_prg, input, output, width, height));
	}
	else if( inputFormat == IMAGE_YV12 )
	{
		if( outputFormat == IMAGE_RGB8 )
			return OCL(oclYV12ToRGB8(YV12_to_RGB_prg, input, output, width, height));
		else if( outputFormat == IMAGE_RGB32F )
			return OCL(oclYV12ToRGB32F(YV12_to_RGB_prg, input, output, width, height));
		else if( outputFormat == IMAGE_RGBA8 )
			return OCL(oclYV12ToRGBA8(YV12_to_RGB_prg, input, output, width, height));
		else if( outputFormat == IMAGE_RGBA32F )
			return OCL(oclYV12ToRGBA32F(YV12_to_RGB_prg, input, output, width, height));
	}
	else if( inputFormat == IMAGE_YUYV )
	{
		if( outputFormat == IMAGE_RGB8 )
			return OCL(oclYUYVToRGB8(YUYV_to_RGB_prg, input, output, width, height));
		else if( outputFormat == IMAGE_RGB32F )
			return OCL(oclYUYVToRGB32F(YUYV_to_RGB_prg, input, output, width, height));
		else if( outputFormat == IMAGE_RGBA8 )
			return OCL(oclYUYVToRGBA8(YUYV_to_RGB_prg, input, output, width, height));
		else if( outputFormat == IMAGE_RGBA32F )
			return OCL(oclYUYVToRGBA32F(YUYV_to_RGB_prg, input, output, width, height));
	}
	else if( inputFormat == IMAGE_YVYU )
	{
#if 0
		if( outputFormat == IMAGE_RGB8 )
			return CUDA(cudaYVYUToRGB(input, (uchar3*)output, width, height));
		else if( outputFormat == IMAGE_RGB32F )
			return CUDA(cudaYVYUToRGB(input, (float3*)output, width, height));
		else if( outputFormat == IMAGE_RGBA8 )
			return CUDA(cudaYVYUToRGBA(input, (uchar4*)output, width, height));
		else if( outputFormat == IMAGE_RGBA32F )
			return CUDA(cudaYVYUToRGBA(input, (float4*)output, width, height));
#endif
	}
	else if( inputFormat == IMAGE_UYVY )
	{
		if( outputFormat == IMAGE_RGB8 )
			return OCL(oclUYVYToRGB8(UYVY_to_RGB_prg, input, output, width, height));
		else if( outputFormat == IMAGE_RGB32F )
			return OCL(oclUYVYToRGB32F(UYVY_to_RGB_prg, input, output, width, height));
		else if( outputFormat == IMAGE_RGBA8 )
			return OCL(oclUYVYToRGBA8(UYVY_to_RGB_prg, input, output, width, height));
		else if( outputFormat == IMAGE_RGBA32F )
			return OCL(oclUYVYToRGBA32F(UYVY_to_RGB_prg, input, output, width, height));
	}
	else if( inputFormat == IMAGE_RGB8 )
	{
		if( outputFormat == IMAGE_RGB8 )
			return OCL(clEnqueueCopyBuffer(ocl_get_queue(), input, output, 0, 0, imageFormatSize(inputFormat, width, height), 0, nullptr, nullptr));
#if 0
		else if( outputFormat == IMAGE_RGBA8 )
			return CUDA(cudaRGB8ToRGBA8((uchar3*)input, (uchar4*)output, width, height));
		else if( outputFormat == IMAGE_RGB32F )
			return CUDA(cudaRGB8ToRGB32((uchar3*)input, (float3*)output, width, height));
		else if( outputFormat == IMAGE_RGBA32F )
			return CUDA(cudaRGB8ToRGBA32((uchar3*)input, (float4*)output, width, height));
		else if( outputFormat == IMAGE_BGR8 )
			return CUDA(cudaRGB8ToBGR8((uchar3*)input, (uchar3*)output, width, height));
		else if( outputFormat == IMAGE_BGRA8 )
			return CUDA(cudaRGB8ToRGBA8((uchar3*)input, (uchar4*)output, width, height, true));
		else if( outputFormat == IMAGE_BGR32F )
			return CUDA(cudaRGB8ToRGB32((uchar3*)input, (float3*)output, width, height, true));
		else if( outputFormat == IMAGE_BGRA32F )
			return CUDA(cudaRGB8ToRGBA32((uchar3*)input, (float4*)output, width, height, true));
		else if( outputFormat == IMAGE_GRAY8 )
			return CUDA(cudaRGB8ToGray8((uchar3*)input, (uint8_t*)output, width, height));
		else if( outputFormat == IMAGE_GRAY32F )
			return CUDA(cudaRGB8ToGray32((uchar3*)input, (float*)output, width, height));
#endif
		else if( outputFormat == IMAGE_I420 )
			return OCL(oclRGB8ToI420(RGB_to_I420_prg, input, output, width, height));
#if 0
		else if( outputFormat == IMAGE_YV12 )
			return OCL(oclRGB8ToYV12((uchar3*)input, output, width, height));
#endif
	}
	else if( inputFormat == IMAGE_RGBA8 )
	{
#if 0
		if( outputFormat == IMAGE_RGB8 )
			return CUDA(cudaRGBA8ToRGB8((uchar4*)input, (uchar3*)output, width, height));
#endif
		/*else*/ if( outputFormat == IMAGE_RGBA8 )
			return OCL(clEnqueueCopyBuffer(ocl_get_queue(), input, output, 0, 0, imageFormatSize(inputFormat, width, height), 0, nullptr, nullptr));
#if 0
		else if( outputFormat == IMAGE_RGB32F )
			return CUDA(cudaRGBA8ToRGB32((uchar4*)input, (float3*)output, width, height));
		else if( outputFormat == IMAGE_RGBA32F )
			return CUDA(cudaRGBA8ToRGBA32((uchar4*)input, (float4*)output, width, height));
		else if( outputFormat == IMAGE_BGR8 )
			return CUDA(cudaRGBA8ToRGB8((uchar4*)input, (uchar3*)output, width, height, true));
		else if( outputFormat == IMAGE_BGRA8 )
			return CUDA(cudaRGBA8ToBGRA8((uchar4*)input, (uchar4*)output, width, height));
		else if( outputFormat == IMAGE_BGR32F )
			return CUDA(cudaRGBA8ToRGB32((uchar4*)input, (float3*)output, width, height, true));
		else if( outputFormat == IMAGE_BGRA32F )
			return CUDA(cudaRGBA8ToRGBA32((uchar4*)input, (float4*)output, width, height, true));
		else if( outputFormat == IMAGE_GRAY8 )
			return CUDA(cudaRGBA8ToGray8((uchar4*)input, (uint8_t*)output, width, height));
		else if( outputFormat == IMAGE_GRAY32F )
			return CUDA(cudaRGBA8ToGray32((uchar4*)input, (float*)output, width, height));
#endif
		else if( outputFormat == IMAGE_I420 )
			return OCL(oclRGBA8ToI420(RGB_to_I420_prg, input, output, width, height));
#if 0
		else if( outputFormat == IMAGE_YV12 )
			return CUDA(cudaRGBAToYV12((uchar4*)input, output, width, height));
#endif
	}
#if 0
	else if( inputFormat == IMAGE_RGB32F )
	{
		if( outputFormat == IMAGE_RGB8 )
			return CUDA(cudaRGB32ToRGB8((float3*)input, (uchar3*)output, width, height, false, pixel_range));	
		else if( outputFormat == IMAGE_RGBA8 )
			return CUDA(cudaRGB32ToRGBA8((float3*)input, (uchar4*)output, width, height, false, pixel_range));	
		else if( outputFormat == IMAGE_RGB32F )
			return CUDA(cudaMemcpy(output, input, imageFormatSize(inputFormat, width, height), cudaMemcpyDeviceToDevice));
		else if( outputFormat == IMAGE_RGBA32F )
			return CUDA(cudaRGB32ToRGBA32((float3*)input, (float4*)output, width, height));
		else if( outputFormat == IMAGE_BGR8 )
			return CUDA(cudaRGB32ToRGB8((float3*)input, (uchar3*)output, width, height, true, pixel_range));	
		else if( outputFormat == IMAGE_BGRA8 )
			return CUDA(cudaRGB32ToRGBA8((float3*)input, (uchar4*)output, width, height, true, pixel_range));	
		else if( outputFormat == IMAGE_BGR32F )
			return CUDA(cudaRGB32ToBGR32((float3*)input, (float3*)output, width, height));
		else if( outputFormat == IMAGE_BGRA32F )
			return CUDA(cudaRGB32ToRGBA32((float3*)input, (float4*)output, width, height, true));
		else if( outputFormat == IMAGE_GRAY8 )
			return CUDA(cudaRGB32ToGray8((float3*)input, (uint8_t*)output, width, height, false, pixel_range));
		else if( outputFormat == IMAGE_GRAY32F )
			return CUDA(cudaRGB32ToGray32((float3*)input, (float*)output, width, height));
		else if( outputFormat == IMAGE_I420 )
			return CUDA(cudaRGBToI420((float3*)input, output, width, height));
		else if( outputFormat == IMAGE_YV12 )
			return CUDA(cudaRGBToYV12((float3*)input, output, width, height));
	}
	else if( inputFormat == IMAGE_RGBA32F )
	{
		if( outputFormat == IMAGE_RGB8 )
			return CUDA(cudaRGBA32ToRGB8((float4*)input, (uchar3*)output, width, height, false, pixel_range));	
		else if( outputFormat == IMAGE_RGBA8 )
			return CUDA(cudaRGBA32ToRGBA8((float4*)input, (uchar4*)output, width, height, false, pixel_range));	
		else if( outputFormat == IMAGE_RGB32F )
			return CUDA(cudaRGBA32ToRGB32((float4*)input, (float3*)output, width, height));
		else if( outputFormat == IMAGE_RGBA32F )
			return CUDA(cudaMemcpy(output, input, imageFormatSize(inputFormat, width, height), cudaMemcpyDeviceToDevice));
		else if( outputFormat == IMAGE_BGR8 )
			return CUDA(cudaRGBA32ToRGB8((float4*)input, (uchar3*)output, width, height, true, pixel_range));	
		else if( outputFormat == IMAGE_BGRA8 )
			return CUDA(cudaRGBA32ToRGBA8((float4*)input, (uchar4*)output, width, height, true, pixel_range));	
		else if( outputFormat == IMAGE_BGR32F )
			return CUDA(cudaRGBA32ToRGB32((float4*)input, (float3*)output, width, height, true));
		else if( outputFormat == IMAGE_BGRA32F )
			return CUDA(cudaRGBA32ToBGRA32((float4*)input, (float4*)output, width, height));
		else if( outputFormat == IMAGE_GRAY8 )
			return CUDA(cudaRGBA32ToGray8((float4*)input, (uint8_t*)output, width, height, false, pixel_range));
		else if( outputFormat == IMAGE_GRAY32F )
			return CUDA(cudaRGBA32ToGray32((float4*)input, (float*)output, width, height));
		else if( outputFormat == IMAGE_I420 )
			return CUDA(cudaRGBAToI420((float4*)input, output, width, height));
		else if( outputFormat == IMAGE_YV12 )
			return CUDA(cudaRGBAToYV12((float4*)input, output, width, height));
	}
	else if( inputFormat == IMAGE_BGR8 )
	{
		if( outputFormat == IMAGE_BGR8 )
			return CUDA(cudaMemcpy(output, input, imageFormatSize(inputFormat, width, height), cudaMemcpyDeviceToDevice));
		else if( outputFormat == IMAGE_BGR8 )
			return CUDA(cudaRGB8ToRGBA8((uchar3*)input, (uchar4*)output, width, height));
		else if( outputFormat == IMAGE_BGR32F )
			return CUDA(cudaRGB8ToRGB32((uchar3*)input, (float3*)output, width, height));
		else if( outputFormat == IMAGE_BGRA32F )
			return CUDA(cudaRGB8ToRGBA32((uchar3*)input, (float4*)output, width, height));
		else if( outputFormat == IMAGE_RGB8 )
			return CUDA(cudaRGB8ToBGR8((uchar3*)input, (uchar3*)output, width, height));
		else if( outputFormat == IMAGE_RGBA8 )
			return CUDA(cudaRGB8ToRGBA8((uchar3*)input, (uchar4*)output, width, height, true));
		else if( outputFormat == IMAGE_RGB32F )
			return CUDA(cudaRGB8ToRGB32((uchar3*)input, (float3*)output, width, height, true));
		else if( outputFormat == IMAGE_RGBA32F )
			return CUDA(cudaRGB8ToRGBA32((uchar3*)input, (float4*)output, width, height, true));
		else if( outputFormat == IMAGE_GRAY8 )
			return CUDA(cudaRGB8ToGray8((uchar3*)input, (uint8_t*)output, width, height, true));
		else if( outputFormat == IMAGE_GRAY32F )
			return CUDA(cudaRGB8ToGray32((uchar3*)input, (float*)output, width, height, true));
	}
	else if( inputFormat == IMAGE_BGRA8 )
	{
		if( outputFormat == IMAGE_BGR8 )
			return CUDA(cudaRGBA8ToRGB8((uchar4*)input, (uchar3*)output, width, height));
		else if( outputFormat == IMAGE_BGRA8 )
			return CUDA(cudaMemcpy(output, input, imageFormatSize(inputFormat, width, height), cudaMemcpyDeviceToDevice));
		else if( outputFormat == IMAGE_BGR32F )
			return CUDA(cudaRGBA8ToRGB32((uchar4*)input, (float3*)output, width, height));
		else if( outputFormat == IMAGE_BGRA32F )
			return CUDA(cudaRGBA8ToRGBA32((uchar4*)input, (float4*)output, width, height));
		else if( outputFormat == IMAGE_RGB8 )
			return CUDA(cudaRGBA8ToRGB8((uchar4*)input, (uchar3*)output, width, height, true));
		else if( outputFormat == IMAGE_RGBA8 )
			return CUDA(cudaRGBA8ToBGRA8((uchar4*)input, (uchar4*)output, width, height));
		else if( outputFormat == IMAGE_RGB32F )
			return CUDA(cudaRGBA8ToRGB32((uchar4*)input, (float3*)output, width, height, true));
		else if( outputFormat == IMAGE_RGBA32F )
			return CUDA(cudaRGBA8ToRGBA32((uchar4*)input, (float4*)output, width, height, true));
		else if( outputFormat == IMAGE_GRAY8 )
			return CUDA(cudaRGBA8ToGray8((uchar4*)input, (uint8_t*)output, width, height, true));
		else if( outputFormat == IMAGE_GRAY32F )
			return CUDA(cudaRGBA8ToGray32((uchar4*)input, (float*)output, width, height, true));
	}
	else if( inputFormat == IMAGE_BGR32F )
	{
		if( outputFormat == IMAGE_BGR8 )
			return CUDA(cudaRGB32ToRGB8((float3*)input, (uchar3*)output, width, height, false, pixel_range));	
		else if( outputFormat == IMAGE_BGRA8 )
			return CUDA(cudaRGB32ToRGBA8((float3*)input, (uchar4*)output, width, height, false, pixel_range));	
		else if( outputFormat == IMAGE_BGR32F )
			return CUDA(cudaMemcpy(output, input, imageFormatSize(inputFormat, width, height), cudaMemcpyDeviceToDevice));
		else if( outputFormat == IMAGE_BGRA32F )
			return CUDA(cudaRGB32ToRGBA32((float3*)input, (float4*)output, width, height));
		else if( outputFormat == IMAGE_RGB8 )
			return CUDA(cudaRGB32ToRGB8((float3*)input, (uchar3*)output, width, height, true, pixel_range));	
		else if( outputFormat == IMAGE_RGBA8 )
			return CUDA(cudaRGB32ToRGBA8((float3*)input, (uchar4*)output, width, height, true, pixel_range));	
		else if( outputFormat == IMAGE_RGB32F )
			return CUDA(cudaRGB32ToBGR32((float3*)input, (float3*)output, width, height));
		else if( outputFormat == IMAGE_RGBA32F )
			return CUDA(cudaRGB32ToRGBA32((float3*)input, (float4*)output, width, height, true));
		else if( outputFormat == IMAGE_GRAY8 )
			return CUDA(cudaRGB32ToGray8((float3*)input, (uint8_t*)output, width, height, true, pixel_range));
		else if( outputFormat == IMAGE_GRAY32F )
			return CUDA(cudaRGB32ToGray32((float3*)input, (float*)output, width, height, true));
	}
	else if( inputFormat == IMAGE_BGRA32F )
	{
		if( outputFormat == IMAGE_BGR8 )
			return CUDA(cudaRGBA32ToRGB8((float4*)input, (uchar3*)output, width, height, false, pixel_range));	
		else if( outputFormat == IMAGE_BGRA8 )
			return CUDA(cudaRGBA32ToRGBA8((float4*)input, (uchar4*)output, width, height, false, pixel_range));	
		else if( outputFormat == IMAGE_BGR32F )
			return CUDA(cudaRGBA32ToRGB32((float4*)input, (float3*)output, width, height));
		else if( outputFormat == IMAGE_BGRA32F )
			return CUDA(cudaMemcpy(output, input, imageFormatSize(inputFormat, width, height), cudaMemcpyDeviceToDevice));
		else if( outputFormat == IMAGE_RGB8 )
			return CUDA(cudaRGBA32ToRGB8((float4*)input, (uchar3*)output, width, height, true, pixel_range));	
		else if( outputFormat == IMAGE_RGBA8 )
			return CUDA(cudaRGBA32ToRGBA8((float4*)input, (uchar4*)output, width, height, true, pixel_range));	
		else if( outputFormat == IMAGE_RGB32F )
			return CUDA(cudaRGBA32ToRGB32((float4*)input, (float3*)output, width, height, true));
		else if( outputFormat == IMAGE_RGBA32F )
			return CUDA(cudaRGBA32ToBGRA32((float4*)input, (float4*)output, width, height));
		else if( outputFormat == IMAGE_GRAY8 )
			return CUDA(cudaRGBA32ToGray8((float4*)input, (uint8_t*)output, width, height, true, pixel_range));
		else if( outputFormat == IMAGE_GRAY32F )
			return CUDA(cudaRGBA32ToGray32((float4*)input, (float*)output, width, height, true));		
	}
	else if( inputFormat == IMAGE_GRAY8 )
	{
		if( outputFormat == IMAGE_RGB8 || outputFormat == IMAGE_BGR8 )
			return CUDA(cudaGray8ToRGB8((uint8_t*)input, (uchar3*)output, width, height));
		else if( outputFormat == IMAGE_RGBA8 || outputFormat == IMAGE_BGRA8 )
			return CUDA(cudaGray8ToRGBA8((uint8_t*)input, (uchar4*)output, width, height));
		else if( outputFormat == IMAGE_RGB32F || outputFormat == IMAGE_BGR32F )
			return CUDA(cudaGray8ToRGB32((uint8_t*)input, (float3*)output, width, height));
		else if( outputFormat == IMAGE_RGBA32F || outputFormat == IMAGE_BGRA32F )
			return CUDA(cudaGray8ToRGBA32((uint8_t*)input, (float4*)output, width, height));
		else if( outputFormat == IMAGE_GRAY8 )
			return CUDA(cudaMemcpy(output, input, imageFormatSize(inputFormat, width, height), cudaMemcpyDeviceToDevice));
		else if( outputFormat == IMAGE_GRAY32F )
			return CUDA(cudaGray8ToGray32((uint8_t*)input, (float*)output, width, height));
	}
	else if( inputFormat == IMAGE_GRAY32F )
	{
		if( outputFormat == IMAGE_RGB8 || outputFormat == IMAGE_BGR8 )
			return CUDA(cudaGray32ToRGB8((float*)input, (uchar3*)output, width, height, pixel_range));
		else if( outputFormat == IMAGE_RGBA8 || outputFormat == IMAGE_BGRA8 )
			return CUDA(cudaGray32ToRGBA8((float*)input, (uchar4*)output, width, height, pixel_range));
		else if( outputFormat == IMAGE_RGB32F || outputFormat == IMAGE_BGR32F )
			return CUDA(cudaGray32ToRGB32((float*)input, (float3*)output, width, height));
		else if( outputFormat == IMAGE_RGBA32F || outputFormat == IMAGE_BGRA32F )
			return CUDA(cudaGray32ToRGBA32((float*)input, (float4*)output, width, height));
		else if( outputFormat == IMAGE_GRAY8 )
			return CUDA(cudaGray32ToGray8((float*)input, (uint8_t*)output, width, height, pixel_range));
		else if( outputFormat == IMAGE_GRAY32F )
			return CUDA(cudaMemcpy(output, input, imageFormatSize(inputFormat, width, height), cudaMemcpyDeviceToDevice));
	}
	else if( imageFormatIsBayer(inputFormat) )
	{
		if( outputFormat == IMAGE_RGB8 )
			return CUDA(cudaBayerToRGB((uint8_t*)input, (uchar3*)output, width, height, inputFormat));
		else if( outputFormat == IMAGE_RGBA8 )
			return CUDA(cudaBayerToRGBA((uint8_t*)input, (uchar3*)output, width, height, inputFormat));
	}
#endif

	LogError(LOG_OCL "oclColorConvert() -- invalid input/output format combination (%s -> %s)\n", imageFormatToStr(inputFormat), imageFormatToStr(outputFormat));
	return CL_INVALID_VALUE;
}


