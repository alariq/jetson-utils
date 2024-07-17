#include "oclYV12.h"
#include "imageFormat.h"
#include "timespec.h"

#include "ocl_utils.h"
#include "ocl_program.h"
#include "CL/cl.h"

#include <cassert>

template<int NCH, imageFormat format>
static cl_int launchNV12ToRGB(ocl_program& prg, cl_mem input, cl_mem output, size_t width, size_t height, bool b_float_output)
{
	if( !input || !output || !width || !height )
		return CL_INVALID_VALUE;

	const int srcPitch = width * sizeof(uint8_t);
	const int dstPitch = width * (b_float_output?sizeof(float): 1);

	const int wg_size_x = 32;
	const int wg_size_y = 8;
	const int num_blocks[2] = { iDivUp(width, wg_size_x), iDivUp(height, wg_size_y)};
	const int num_channels = NCH;

	const char* k_name = "NV12ToRGB";
	const cl_kernel k = prg.kernels_[k_name];

	cl_int status;

	status = clSetKernelArg(k, 0, sizeof(cl_mem), &input);    CHECK_OPENCL_ERROR(status, "clSetKernelArg failed.");
	status = clSetKernelArg(k, 1, sizeof(cl_int), &srcPitch);    CHECK_OPENCL_ERROR(status, "clSetKernelArg failed.");
	status = clSetKernelArg(k, 2, sizeof(cl_mem), &output);   CHECK_OPENCL_ERROR(status, "clSetKernelArg failed.");
	status = clSetKernelArg(k, 3, sizeof(cl_int), &dstPitch);   CHECK_OPENCL_ERROR(status, "clSetKernelArg failed.");
	status = clSetKernelArg(k, 4, sizeof(cl_int), &width);CHECK_OPENCL_ERROR(status, "clSetKernelArg failed.");
	status = clSetKernelArg(k, 5, sizeof(cl_int), &height);   CHECK_OPENCL_ERROR(status, "clSetKernelArg failed.");
	status = clSetKernelArg(k, 6, sizeof(cl_int), &num_channels);   CHECK_OPENCL_ERROR(status, "clSetKernelArg failed.");

    cl_event event;
    if(!ocl_execute_kernel_2d(k, wg_size_x, wg_size_y, num_blocks[0], num_blocks[1], 0, nullptr, &event)) {
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

        LogInfo("[GPU] kernel exec time: %f\n", exec_time);
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

cl_int oclNV12ToRGB8(ocl_program& prg, cl_mem input, cl_mem output, size_t width, size_t height)
{
	return launchNV12ToRGB<3, IMAGE_NV12>(prg, input, output, width, height, false);
}

cl_int oclNV12ToRGB32F(ocl_program& prg, cl_mem input, cl_mem output, size_t width, size_t height)
{
	return launchNV12ToRGB<3, IMAGE_NV12>(prg, input, output, width, height, false);
}

cl_int oclNV12ToRGBA8(ocl_program& prg, cl_mem input, cl_mem output, size_t width, size_t height)
{
	return launchNV12ToRGB<4, IMAGE_NV12>(prg, input, output, width, height, false);
}

cl_int oclNV12ToRGBA32F(ocl_program& prg, cl_mem input, cl_mem output, size_t width, size_t height)
{
	return launchNV12ToRGB<4, IMAGE_NV12>(prg, input, output, width, height, false);
}

