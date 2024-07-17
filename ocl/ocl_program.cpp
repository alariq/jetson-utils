#include <cassert>
#include <stdio.h>
#include <cstring>
#include <CL/cl.h>
#include "ocl_utils.h"
#include "ocl_program.h"
#include "utils/stream.h"

const char* ocl_load(const char* fname)
{
    assert(fname);
    stream* pstream = stream::makeFileStream();
    if(0 != pstream->open(fname,"rb"))
    {
		delete pstream;
        printf("Can't open %s \n", fname);
        return 0;
    }

    pstream->seek(0, stream::S_END);
    size_t size = pstream->tell();
    pstream->seek(0, stream::S_SET);

    char* pdata = new char[size + 1];
    size_t rv = pstream->read(pdata, 1, size);
    assert(rv==size);
    pdata[size] = '\0';

	delete pstream;
    return pdata;
}

bool checkBuildStatus(cl_int status, cl_program program, cl_device_id* devices)
{
	//if(status != CL_SUCCESS)
    {
        //if(status == CL_BUILD_PROGRAM_FAILURE)
        {
            cl_int logStatus;
            char *buildLog = NULL;
            size_t buildLogSize = 0;
            logStatus = clGetProgramBuildInfo (
                            program, 
                            devices[0],
                            CL_PROGRAM_BUILD_LOG, 
                            buildLogSize, 
                            buildLog, 
                            &buildLogSize);
            CHECK_OPENCL_ERROR(logStatus, "clGetProgramBuildInfo failed.");

            buildLog = (char*)malloc(buildLogSize);

            memset(buildLog, 0, buildLogSize);

            logStatus = clGetProgramBuildInfo (
                            program, 
                            devices[0], 
                            CL_PROGRAM_BUILD_LOG, 
                            buildLogSize, 
                            buildLog, 
                            NULL);
            if(false == ocl_check_val(logStatus, CL_SUCCESS, "clGetProgramBuildInfo failed."))
            {
                free(buildLog);
                return false;
            }
			if(strlen(buildLog))
			{
				printf(" \n\t\t\tBUILD LOG\n");
				printf(" ************************************************\n");
				printf("%s\n", buildLog);
				printf(" ************************************************\n");
			}
            free(buildLog);
        }

        CHECK_OPENCL_ERROR(status, "clBuildProgram failed.");
    }

	return true;
}

bool ocl_program::load(const char* fname, const char* flags, cl_context ctx, cl_device_id* devices, int num_devices)
{
	assert(fname);

	fname_ = fname;
	if(flags)
		flags_ = flags;
	devices_ = devices;
	num_devices_ = num_devices;
	context_ = ctx;

	const char* psource = ocl_load(fname);
	if(!psource)
        return false;
	cl_int status;

	program_ = clCreateProgramWithSource(ctx, 1, &psource, 0, &status);
    CHECK_OPENCL_ERROR(status, "clCreateProgramWithSource failed.");

	status = clBuildProgram(program_, num_devices, devices, flags, NULL, NULL);
	bool is_success = checkBuildStatus(status, program_, devices);
	if(!is_success)
	{
		program_ = 0;
	}
	return is_success;
}

bool ocl_program::reload()
{
    destroy();

	Kernels_t::iterator it = kernels_.begin();
	Kernels_t::iterator end = kernels_.end();
	if(load(fname_.c_str(), flags_.c_str(), context_, devices_, num_devices_))
	{
		// reload all kernels
		it = kernels_.begin();
		for(;it!=end;++it)
		{
			create_kernel(it->first.c_str());
		}
		return true;
	}
	else
	{
		program_ = 0;
	}

	return false;
}

void ocl_program::destroy()
{
	Kernels_t::iterator it = kernels_.begin();
	Kernels_t::iterator end = kernels_.end();
	if(program_)
	{
		for(;it!=end;++it)
		{
			if(it->second)
			{
				clReleaseKernel(it->second);
				it->second = 0;
			}
		}
		clReleaseProgram(program_);
	}
    program_= NULL;
}

bool ocl_program::create_kernel(const char* kname)
{
	assert(program_);
	
	cl_int status;
	cl_kernel kernel = clCreateKernel(program_, kname, &status);
	CHECK_OPENCL_ERROR(status, "clCreateKernel failed.");
	
	if(0 == kernels_.count(kname) || kernels_[kname] == 0)
	{
		kernels_[kname] = kernel;
	}
	else
	{
		clReleaseKernel(kernels_[kname]);
		kernels_[kname] = kernel;
	}

	return true;
}

bool ocl_program::load_binary(const char* flags, cl_context ctx, cl_device_id* devices, int num_devices, size_t* sizes, const unsigned char** binaries)
{
	if(flags)
		flags_ = flags;
	devices_ = devices;
	num_devices_ = num_devices;
	context_ = ctx;
    
	cl_int status;
	cl_int* bin_status = new cl_int[num_devices];
	program_ = clCreateProgramWithBinary(ctx, num_devices, devices, sizes, binaries, bin_status, &status);
	CHECK_OPENCL_ERROR(status, "clCreateProgramWithBinary failed.");
	// check that program was successfully created for all requested devices
	for(int i=0;i<num_devices;++i)
	{
		CHECK_OPENCL_ERROR(bin_status[i], "clCreateProgramWithBinary failed.");
	}

	// NB: have to comment this for test to avoid crash :-(
	//delete[] bin_status;

	status = clBuildProgram(program_, num_devices, devices, flags, NULL, NULL);
	return checkBuildStatus(status, program_, devices);
}


bool ocl_program::get_binary(unsigned char**& pout, size_t** sizes, int* num)
{
	if(!program_) return false;
	
	cl_int status;
	cl_uint numdev;
	status = clGetProgramInfo(program_, CL_PROGRAM_NUM_DEVICES, sizeof(cl_uint), &numdev, NULL);
	CHECK_OPENCL_ERROR(status, "clGetProgramInfo failed.");

	size_t* mysizes = new size_t[numdev];

	status = clGetProgramInfo(program_, CL_PROGRAM_BINARY_SIZES, sizeof(size_t)*numdev, mysizes, NULL);
	CHECK_OPENCL_ERROR(status, "clGetProgramInfo failed.");
	
	unsigned char** pbin = new unsigned char*[numdev];
	for(size_t i=0; i< numdev; ++i)
	{
		pbin[i] = new unsigned char[mysizes[i]];
	}

	cl_device_id* pids = new cl_device_id[numdev];
	status = clGetProgramInfo(program_, CL_PROGRAM_DEVICES, sizeof(cl_device_id)*numdev, pids, NULL);
	CHECK_OPENCL_ERROR(status, "clGetProgramInfo failed.");
	//delete pids;

	status = clGetProgramInfo(program_, CL_PROGRAM_BINARIES, sizeof(unsigned char*)*numdev, pbin, NULL);
	CHECK_OPENCL_ERROR(status, "clGetProgramInfo failed.");


	*num = numdev;
	*sizes = mysizes;
	pout = pbin;
	return true;
}
