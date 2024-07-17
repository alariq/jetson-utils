#include "oclYUYV.h"
#include "imageFormat.h"
#include "timespec.h"

#include "ocl_utils.h"
#include "ocl_program.h"
#include "CL/cl.h"

#include <cassert>

template<int NCH, imageFormat format>
static cl_int launchYUYVToRGB(ocl_program& prg, cl_mem input, cl_mem output, size_t width, size_t height, bool b_float_output)
{
	SCOPED_TIMER("launchYUYVToRGB");

	if( !input || !output || !width || !height )
		return CL_INVALID_VALUE;

	const int halfWidth = width / 2;	// two pixels are output at once
	const int wg_size = 8;
	const int num_blocks[2] = { iDivUp(halfWidth, wg_size), iDivUp(height, wg_size)};
	const int num_channels = NCH;

	const char* k_name = "YUYVToRGB";
	const cl_kernel k = prg.kernels_[k_name];

	cl_int status;

	status = clSetKernelArg(k, 0, sizeof(cl_mem), &input);    CHECK_OPENCL_ERROR(status, "clSetKernelArg failed.");
	status = clSetKernelArg(k, 1, sizeof(cl_mem), &output);   CHECK_OPENCL_ERROR(status, "clSetKernelArg failed.");
	status = clSetKernelArg(k, 2, sizeof(cl_int), &halfWidth);CHECK_OPENCL_ERROR(status, "clSetKernelArg failed.");
	status = clSetKernelArg(k, 3, sizeof(cl_int), &height);   CHECK_OPENCL_ERROR(status, "clSetKernelArg failed.");
	status = clSetKernelArg(k, 4, sizeof(cl_int), &num_channels);   CHECK_OPENCL_ERROR(status, "clSetKernelArg failed.");

    cl_event event;
    if(!ocl_execute_kernel_2d(k, wg_size, wg_size, num_blocks[0], num_blocks[1], 0, nullptr, &event)) {
        return false;
    }

    bool b_enable_time_stats = false;

    if(b_enable_time_stats) {

        clFlush(ocl_get_queue());

        {
            //SCOPED_TIMER("crop wait for event");
            ocl_wait_for_event(&event, false);
        }

        float exec_time;
        ocl_get_exec_time_in_ms(&event, &exec_time);

        // do not release because we return this event
        //clReleaseEvent(event);

        LogInfo("[GPU] crop Total exec time: %f\n", exec_time);
    }

	if(status == CL_SUCCESS)
	{
		//SCOPED_TIMER("getSbuwindow clWaitForEvents");
		status = clWaitForEvents(1, &event);
		CHECK_OPENCL_ERROR_NORET(status, "clWaitForEvents Failed with Error Code:");
		clReleaseEvent(event);
	}


	return status;
}

/**
 * Convert a YUV I420 planar image to RGB uchar3.
 */
cl_int oclYUYVToRGB8(ocl_program& prg, cl_mem input, cl_mem output, size_t width, size_t height)
{
	return launchYUYVToRGB<3, IMAGE_YUYV>(prg, input, output, width, height, false);
}

cl_int oclYUYVToRGB32F(ocl_program& prg, cl_mem input, cl_mem output, size_t width, size_t height)
{
	return launchYUYVToRGB<3, IMAGE_YUYV>(prg, input, output, width, height, true);
}

/**
 * Convert a YUV YV12 planar image to RGBA uchar4.
 */
cl_int oclYUYVToRGBA8(ocl_program& prg, cl_mem input, cl_mem output, size_t width, size_t height)
{
	return launchYUYVToRGB<4, IMAGE_YUYV>(prg, input, output, width, height, false);
}

/**
 * Convert a YUV YV12 planar image to RGB float4.
 */
cl_int oclYUYVToRGBA32F(ocl_program& prg, cl_mem input, cl_mem output, size_t width, size_t height)
{
	return launchYUYVToRGB<4, IMAGE_YUYV>(prg, input, output, width, height, true);
}


// cudaUYVYToRGB (uchar3)
cl_int oclUYVYToRGB8(ocl_program& prg, cl_mem input, cl_mem output, size_t width, size_t height )
{
	return launchYUYVToRGB<3, IMAGE_UYVY>(prg, input, output, width, height, false);
}

// oclUYVYToRGB (float3)
cl_int oclUYVYToRGB32F(ocl_program& prg,  cl_mem input, cl_mem output, size_t width, size_t height )
{
	return launchYUYVToRGB<3, IMAGE_UYVY>(prg, input, output, width, height, true);
}

// oclUYVYToRGBA (uchar4)
cl_int oclUYVYToRGBA8(ocl_program& prg,  cl_mem input, cl_mem output, size_t width, size_t height )
{
	return launchYUYVToRGB<4, IMAGE_UYVY>(prg, input, output, width, height, false);
}

// oclUYVYToRGBA (float4)
cl_int oclUYVYToRGBA32F(ocl_program& prg,  cl_mem input, cl_mem output, size_t width, size_t height )
{
	return launchYUYVToRGB<4, IMAGE_UYVY>(prg, input, output, width, height, true);
}

