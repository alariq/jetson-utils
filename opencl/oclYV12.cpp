#include "oclYV12.h"
#include "imageFormat.h"
#include "timespec.h"

#include "ocl_utils.h"
#include "ocl_program.h"
#include "CL/cl.h"

#include <cassert>

template<int NCH, imageFormat format>
static cl_int launchI420ToRGB(ocl_program& prg, cl_mem input, cl_mem output, size_t width, size_t height, bool b_float_output)
{
	SCOPED_TIMER("launchI420ToRGB");

	if( !input || !output || !width || !height )
		return CL_INVALID_VALUE;

	const int srcPitch = width * sizeof(uint8_t);
	const int dstPitch = width * (b_float_output?sizeof(float): 1);

	const int wg_size = 8;
	const int num_blocks[2] = { iDivUp(width, wg_size), iDivUp(height, wg_size)};
	const int num_channels = NCH;

	const char* k_name = "I420ToRGB";
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

cl_int oclI420ToRGB8(ocl_program& prg, cl_mem input, cl_mem output, size_t width, size_t height)
{
	return launchI420ToRGB<3, IMAGE_I420>(prg, input, output, width, height, false);
}

cl_int oclI420ToRGB32F(ocl_program& prg, cl_mem input, cl_mem output, size_t width, size_t height)
{
	return launchI420ToRGB<3, IMAGE_I420>(prg, input, output, width, height, true);
}

cl_int oclI420ToRGBA8(ocl_program& prg, cl_mem input, cl_mem output, size_t width, size_t height)
{
	return launchI420ToRGB<4, IMAGE_I420>(prg, input, output, width, height, false);
}

cl_int oclI420ToRGBA32F(ocl_program& prg, cl_mem input, cl_mem output, size_t width, size_t height)
{
	return launchI420ToRGB<4, IMAGE_I420>(prg, input, output, width, height, true);
}


cl_int oclYV12ToRGB8(ocl_program& prg, cl_mem input, cl_mem output, size_t width, size_t height)
{
	return launchI420ToRGB<3, IMAGE_YV12>(prg, input, output, width, height, false);
}

cl_int oclYV12ToRGB32F(ocl_program& prg, cl_mem input, cl_mem output, size_t width, size_t height)
{
	return launchI420ToRGB<3, IMAGE_YV12>(prg, input, output, width, height, false);
}

cl_int oclYV12ToRGBA8(ocl_program& prg, cl_mem input, cl_mem output, size_t width, size_t height)
{
	return launchI420ToRGB<4, IMAGE_YV12>(prg, input, output, width, height, false);
}

cl_int oclYV12ToRGBA32F(ocl_program& prg, cl_mem input, cl_mem output, size_t width, size_t height)
{
	return launchI420ToRGB<4, IMAGE_YV12>(prg, input, output, width, height, false);
}

/* -------------------------------------------------------------------------------- */

template<int NCH, bool isYV12>
static cl_int launchRGBTo420(ocl_program& prg, cl_mem input, int i_pitch, cl_mem output, int o_pitch, size_t width, size_t height, bool b_float_input)
{
	SCOPED_TIMER("launchRGBTo420");

	if( !input || !output || !width || !height || !i_pitch || !o_pitch )
		return CL_INVALID_VALUE;

	const int wg_size_x = 32;
	const int wg_size_y = 8;
	const int num_blocks[2] = { iDivUp(width, wg_size_x * 2), iDivUp(height, wg_size_y * 2)};
	const int num_channels = NCH;
	const int inputAlignedWidth = 
		i_pitch / (NCH*(b_float_input?sizeof(float) : sizeof(uint8_t)));

	const char* k_name = "RGBToYV12";
	const cl_kernel k = prg.kernels_[k_name];

	cl_int status;

	status = clSetKernelArg(k, 0, sizeof(cl_mem), &input);    CHECK_OPENCL_ERROR(status, "clSetKernelArg failed.");
	status = clSetKernelArg(k, 1, sizeof(cl_int), &inputAlignedWidth);    CHECK_OPENCL_ERROR(status, "clSetKernelArg failed.");
	status = clSetKernelArg(k, 2, sizeof(cl_mem), &output);   CHECK_OPENCL_ERROR(status, "clSetKernelArg failed.");
	status = clSetKernelArg(k, 3, sizeof(cl_int), &o_pitch);   CHECK_OPENCL_ERROR(status, "clSetKernelArg failed.");
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


// cudaRGBToI420 (uchar3)
cl_int oclRGB8ToI420(ocl_program& p, cl_mem input, int inputPitch, cl_mem output, int outputPitch, int width, int height )
{
	return launchRGBTo420<3, true>(p, input, inputPitch, output, outputPitch, width, height, false );
}

// cudaRGBToI420 (uchar3)
cl_int oclRGB8ToI420(ocl_program& p, cl_mem input, cl_mem output, int width, int height )
{
	return oclRGB8ToI420(p, input, width*sizeof(uint8_t)*3, output, width, width, height );
}

// cudaRGBToI420 (float3)
cl_int oclRGB32FToI420(ocl_program& p, cl_mem input, int inputPitch, cl_mem output, int outputPitch, int width, int height )
{
	return launchRGBTo420<3, true>(p, input, inputPitch, output, outputPitch, width, height, true);
}

// cudaRGBAToI420 (float3)
cl_int oclRGB32FToI420(ocl_program& p, cl_mem input, cl_mem output, int width, int height )
{
	return oclRGB32FToI420(p, input, width * sizeof(float)*3, output, width * sizeof(uint8_t), width, height);
}

// cudaRGBAToI420 (uchar4)
cl_int oclRGBA8ToI420(ocl_program& p, cl_mem input, size_t inputPitch, cl_mem output, size_t outputPitch, size_t width, size_t height )
{
	return launchRGBTo420<4, true>(p, input, inputPitch, output, outputPitch, width, height, false );
}

// cudaRGBAToI420 (uchar4)
cl_int oclRGBA8ToI420(ocl_program& p,  cl_mem input, cl_mem output, size_t width, size_t height )
{
	return oclRGBA8ToI420(p, input, width * sizeof(uint8_t)*4, output, width * sizeof(uint8_t), width, height );
}

// cudaRGBAToI420 (float4)
cl_int oclRGBA32FToI420(ocl_program& p,  cl_mem input, size_t inputPitch, cl_mem output, size_t outputPitch, size_t width, size_t height )
{
	return launchRGBTo420<4,true>(p, input, inputPitch, output, outputPitch, width, height, true );
}

// cudaRGBAToI420 (float4)
cl_int oclRGBA32FToI420(ocl_program& p, cl_mem input, cl_mem output, size_t width, size_t height )
{
	return oclRGBA32FToI420(p, input, width * sizeof(float)*4, output, width * sizeof(uint8_t), width, height );
}

