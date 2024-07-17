#ifdef PLATFORM_WINDOWS
#include <windows.h>
#endif

#include<CL/cl.h>
//#include<CL/cl_ext.h>
//#include<CL/cl_gl.h>

#include <stdio.h>
#include <cstring>
#include <cassert>
#include <cstdlib>
#include "ocl_device.h"
#include "ocl_utils.h"


clImportMemoryARM_fn clImportMemoryARM = 0;

#define ENABLE_DX11_DEFINES

//static const char* const deviceType = "gpu";
static int platformId = 0;
static cl_platform_id g_ocl_platform = 0;
static cl_context context = 0;
static cl_device_id interopDeviceId = 0;
static cl_device_id *devices;              // CL device list
static cl_command_queue commandQueue;      // CL command queue
static unsigned int deviceIndex = 0;              // device number
static CL_DeviceInfo ocl_deviceInfo;
static bool b_quiet_mode = false;

static const bool b_create_dx11_context = false;
static void* g_dx11_device_ptr = 0;

//#ifdef NVIDIA_OCL_SDK
static const char* const sPreferredPlatformName = "NVIDIA Corporation";
//#else
//static const char* const sPreferredPlatformName = "Advanced Micro Devices, Inc.";
//#endif

////////////////////////////////////////////////////////////////////////////////////
typedef CL_API_ENTRY cl_int (CL_API_CALL *clGetGLContextInfoKHR_fn)(
    const cl_context_properties *properties,
    cl_gl_context_info param_name,
    size_t param_value_size,
    void *param_value,
    size_t *param_value_size_ret);


// Rename references to this dynamically linked function to avoid
// collision with static link version
#define clGetGLContextInfoKHR clGetGLContextInfoKHR_proc
static clGetGLContextInfoKHR_fn clGetGLContextInfoKHR = 0;


////////////////////////////////////////////////////////////////////////////////////
// DX11 interop stuff

#ifdef ENABLE_DX11_DEFINES
typedef cl_uint cl_d3d11_device_source_khr;
typedef cl_uint cl_d3d11_device_set_khr;
#define CL_CONTEXT_D3D11_DEVICE_KHR                  0x401D
#define CL_PREFERRED_DEVICES_FOR_D3D11_KHR           0x401B
#define CL_ALL_DEVICES_FOR_D3D11_KHR                 0x401C
#define CL_D3D11_DEVICE_KHR                          0x4019
#define CL_D3D11_DXGI_ADAPTER_KHR                    0x401A

typedef CL_API_ENTRY cl_int (CL_API_CALL *clGetDeviceIDsFromD3D11KHR_fn)(
	cl_platform_id platform, 
	cl_d3d11_device_source_khr d3d_device_source, 
	void *d3d_object, 
	cl_d3d11_device_set_khr d3d_device_set,
	cl_uint num_entries, 
	cl_device_id * devices, 
	cl_uint * num_devices);

#define clGetDeviceIDsFromD3D11KHR clGetDeviceIDsFromD3D11KHR_proc
static clGetDeviceIDsFromD3D11KHR_fn clGetDeviceIDsFromD3D11KHR = 0;

#endif // ENABLE_DX11_DEFINES
////////////////////////////////////////////////////////////////////////////////////

cl_command_queue ocl_get_queue()
{
	return commandQueue;
}

cl_context ocl_get_context()
{
	return context;
}

cl_device_id* ocl_get_devices()
{
	return devices;
}

unsigned int ocl_get_device_id()
{
	return deviceIndex;
}

cl_platform_id ocl_get_platform_id()
{
	return g_ocl_platform;
}

void ocl_set_dx11_device(void* pdevice)
{
	g_dx11_device_ptr = pdevice;
}
void* ocl_get_dx11_device()
{
	return g_dx11_device_ptr;
}



////////////////////////////////////////////////////////////////////////////////////
bool ocl_get_platform(cl_platform_id &platform, int platformId, bool platformIdEnabled)
{
    cl_uint numPlatforms;
    cl_int status = clGetPlatformIDs(0, NULL, &numPlatforms);
    CHECK_OPENCL_ERROR(status, "clGetPlatformIDs failed.");

    if (0 < numPlatforms) 
    {
        cl_platform_id* platforms = new cl_platform_id[numPlatforms];
        status = clGetPlatformIDs(numPlatforms, platforms, NULL);
        CHECK_OPENCL_ERROR(status, "clGetPlatformIDs failed.");
        if(!b_quiet_mode) printf("Found %d platforms\n", numPlatforms);

		if(platformIdEnabled)
        {
            platform = platforms[platformId];
        }
        else
        {
            char platformName[100];
            for (unsigned i = 0; i < numPlatforms; ++i) 
            {
                status = clGetPlatformInfo(platforms[i],
                                           CL_PLATFORM_VENDOR,
                                           sizeof(platformName),
                                           platformName,
                                           NULL);
				CHECK_OPENCL_ERROR(status, "clGetPlatformInfo failed.");
				if(!b_quiet_mode) printf("Platform[%s]\n", platformName);

                platform = platforms[i];
                if (!strncmp(platformName, sPreferredPlatformName, 100)) 
                {
                    break;
                }
            }
            if(!b_quiet_mode) printf("Platform found : %s\n", platformName);
        }
        delete[] platforms;
    }

    if(NULL == platform)
    {
        printf("NULL platform found so Exiting Application.\n");
        return false;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////////
bool ocl_getDx11Devices(cl_device_id **devices)
{
	/* First, get the size of device list data */
    cl_uint deviceListSize = 0;
	int status = 0;

    cl_platform_id platform = ocl_get_platform_id();

	clGetDeviceIDsFromD3D11KHR(platform, CL_D3D11_DEVICE_KHR, g_dx11_device_ptr,CL_PREFERRED_DEVICES_FOR_D3D11_KHR, 0, NULL, &deviceListSize);
	CHECK_OPENCL_ERROR(status, "clGetDeviceIDsFromD3D11KHR failed.");

    (*devices) = (cl_device_id *)malloc(deviceListSize);
    
    /* Now, get the device list data */
	clGetDeviceIDsFromD3D11KHR(platform, CL_D3D11_DEVICE_KHR, g_dx11_device_ptr,CL_PREFERRED_DEVICES_FOR_D3D11_KHR, deviceListSize, *devices, 0);
	CHECK_OPENCL_ERROR(status, "clGetDeviceIDsFromD3D11KHR failed.");

	return true;
}

////////////////////////////////////////////////////////////////////////////////////
bool ocl_getDevices(cl_context &context, cl_device_id **devices, size_t* numDevices)
{
	/* First, get the size of device list data */
    size_t deviceListSize = 0;
	int status = 0;
	status = clGetContextInfo( context, CL_CONTEXT_DEVICES, 0, NULL, &deviceListSize);
    CHECK_OPENCL_ERROR(status, "clGetContextInfo failed.");

    int deviceCount = (int)(deviceListSize / sizeof(cl_device_id));
	if(numDevices)
		*numDevices = deviceCount;
 
    (*devices) = (cl_device_id *)malloc(deviceListSize);
    
    /* Now, get the device list data */
    status = clGetContextInfo(context, CL_CONTEXT_DEVICES, deviceListSize, (*devices), NULL);
    CHECK_OPENCL_ERROR(status, "clGetGetContextInfo failed.");

	return true;
}

////////////////////////////////////////////////////////////////////////////////////
bool ocl_init(const oclInitParams* params, cl_command_queue_properties queue_prop/* = 0*/)
{
	assert(params);
	cl_int status = CL_SUCCESS;

	const cl_device_type dType = params -> device_type_;

	// TODO: instead of selecting predefined platform, select one, but if there are no devices of type dType for the platform
	// then select another available and so on
	cl_platform_id platform = NULL;
	bool retValue = ocl_get_platform(platform, platformId, false);
	if(retValue != true) {
		printf("getPlatform() failed\n");
		return false;
	}
	g_ocl_platform = platform;

	if(!ocl_displayDevices(platform, dType))
		return false;

	retValue = ocl_create_context(params, platform, context, interopDeviceId);

	// Create command queue
	commandQueue = clCreateCommandQueue/*WithProperties*/(
			context,
			interopDeviceId,
			queue_prop,
			&status);
	CHECK_OPENCL_ERROR(status, "clCreateCommandQueue failed.");

	if(false == ocl_deviceInfo.setDeviceInfo(interopDeviceId))
		return false;

	return retValue;

}

////////////////////////////////////////////////////////////////////////////////////
bool ocl_create_context(const oclInitParams* params, cl_platform_id platform, cl_context &context, cl_device_id &interopDevice)
{
    cl_int status;

    //HDC hDC = GetDC(hWnd);
    cl_context_properties properties_gl[] = 
    {
        CL_CONTEXT_PLATFORM, (cl_context_properties) platform,
#ifdef PLATFORM_WINDOWS
        CL_GL_CONTEXT_KHR,   (cl_context_properties) params->glcontext_,
        CL_WGL_HDC_KHR,      (cl_context_properties) GetDC(params->hwnd_),
#else
        CL_GL_CONTEXT_KHR,   (cl_context_properties) params->glcontext_,
        CL_GLX_DISPLAY_KHR,  (cl_context_properties) params->display_,
        //CL_EGL_DISPLAY_KHR,  (cl_context_properties) params->egl_display_,
#endif
        0
    };

    cl_context_properties properties_cpu[] = 
    {
        CL_CONTEXT_PLATFORM, (cl_context_properties) platform,
        0
    };

    static const cl_context_properties properties_gpu_dx11[] = 
    {
        CL_CONTEXT_PLATFORM, (cl_context_properties) platform,
        CL_CONTEXT_D3D11_DEVICE_KHR, (intptr_t)g_dx11_device_ptr,
        0
    };

    if(!clImportMemoryARM && params->b_enable_import_memory_arm)
    {
	    clImportMemoryARM = (clImportMemoryARM_fn) clGetExtensionFunctionAddressForPlatform(platform, "clImportMemoryARM");
	    if (!clImportMemoryARM) 
	    {
		    printf("Failed to query proc address for clImportMemoryARM\n");
		    return false;
	    }
    }

    const cl_device_type device_type = params -> device_type_;

    if(params && params->b_enable_interop )
    {
        if(b_create_dx11_context)
        {
            clGetDeviceIDsFromD3D11KHR = (clGetDeviceIDsFromD3D11KHR_fn)clGetExtensionFunctionAddressForPlatform(ocl_get_platform_id(), "clGetDeviceIDsFromD3D11KHR");
            if(!clGetDeviceIDsFromD3D11KHR) {
                printf("Failed to query proc address for clGetDeviceIDsFromD3D11KHR\n");
                return false;
            }
            if(!ocl_getDx11Devices(&devices)) {
                printf("ocl_getDx11Devices() call failed\n");
                return false;
            }
            deviceIndex = 0;
            interopDeviceId = devices[deviceIndex];
            context = clCreateContext(properties_gpu_dx11, 1, &interopDeviceId, NULL, NULL, &status);
            CHECK_OPENCL_ERROR(status, "clCreateContext failed!!");
        }
        else
        {
            if (!clGetGLContextInfoKHR)
            {
#ifdef NVIDIA_OCL_SDK
                clGetGLContextInfoKHR = (clGetGLContextInfoKHR_fn) clGetExtensionFunctionAddress("clGetGLContextInfoKHR");
#else
                clGetGLContextInfoKHR = (clGetGLContextInfoKHR_fn) clGetExtensionFunctionAddressForPlatform(platform, "clGetGLContextInfoKHR");
#endif
                if (!clGetGLContextInfoKHR) 
                {
                    printf("Failed to query proc address for clGetGLContextInfoKHR\n");
                    return false;
                }
            }

            size_t device_size = 0;
            cl_device_id int_dev[4] = {0};
		  printf("1 p: %p\n", clGetGLContextInfoKHR);

            status = clGetGLContextInfoKHR( properties_gl, 
                    CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR,
				//CL_DEVICES_FOR_GL_CONTEXT_KHR,
                    sizeof(int_dev), 
                    &int_dev, 
                    &device_size);
		  printf("2\n");
#if 0
		  for(int i=0;i<sizeof(int_dev)/sizeof(int_dev[0]);++i) {
			  if(int_dev[i])
				  printf("Interop device id: %p\n", int_dev[i]);
		  }
#endif
            interopDevice = int_dev[0];
		  printf("3\n");

            CHECK_OPENCL_ERROR(status, "clGetGLContextInfoKHR failed!!");

            // Create OpenCL context from device's id
            context = clCreateContext(properties_gl, 1, &interopDevice, NULL, NULL, &status);
            size_t ndev;
            if(!ocl_getDevices(context, &devices, &ndev))
                return false;
            //find our interop device in the list of devices, to set deviceIndex (index)
            bool deviceFound = false;
            for(size_t i=0;i<ndev;++i)
            {
                if(devices[i] == interopDevice)
                {
                    deviceIndex = i;
                    deviceFound = true;
                    break;
                }
            }

            if(!deviceFound)
                return false;
        }
    }
    else
    {
        size_t ndev;
        context = clCreateContextFromType(properties_cpu, device_type, NULL, NULL, &status);
        if(!ocl_getDevices(context, &devices, &ndev))
            return false;
        deviceIndex = 0;
        interopDeviceId = devices[deviceIndex];
    }

    CHECK_OPENCL_ERROR(status, "clCreateContext failed!!");
    //printf("Interop Device Id: %d\n", interopDevice);
    return true;
}

////////////////////////////////////////////////////////////////////////////////////
bool ocl_displayDevices(cl_platform_id platform, cl_device_type deviceType)
{
    cl_int status;

    // Get platform name
    char platformVendor[1024];
    status = clGetPlatformInfo(platform, CL_PLATFORM_VENDOR, sizeof(platformVendor), platformVendor, NULL);
    CHECK_OPENCL_ERROR(status, "clGetPlatformInfo failed");
    
    if(!b_quiet_mode) printf("\nSelected Platform Vendor : %s\n", platformVendor);

    // Get number of devices available 
    cl_uint deviceCount = 0;
    status = clGetDeviceIDs(platform, deviceType, 0, NULL, &deviceCount);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs failed");

    cl_device_id* deviceIds = (cl_device_id*)malloc(sizeof(cl_device_id) * deviceCount);

    // Get device ids
    status = clGetDeviceIDs(platform, deviceType, deviceCount, deviceIds, NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs failed");

    // Print device index and device names
    for(cl_uint i = 0; i < deviceCount; ++i)
    {
        char deviceName[1024];
		cl_uint max_read_args = 0;
		cl_ulong max_mem_alloc = 0;
		cl_uint max_constant_args = 0;
		cl_device_fp_config single_fp_config=0;
		status = clGetDeviceInfo(deviceIds[i], CL_DEVICE_NAME, sizeof(deviceName), deviceName, NULL);
        CHECK_OPENCL_ERROR(status, "clGetDeviceInfo failed");

		status = clGetDeviceInfo(deviceIds[i], CL_DEVICE_MAX_READ_IMAGE_ARGS, sizeof(cl_uint), &max_read_args, NULL);
        CHECK_OPENCL_ERROR(status, "clGetDeviceInfo failed");

		status = clGetDeviceInfo(deviceIds[i], CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), &max_mem_alloc, NULL);
        CHECK_OPENCL_ERROR(status, "clGetDeviceInfo failed");

		status = clGetDeviceInfo(deviceIds[i], CL_DEVICE_MAX_CONSTANT_ARGS, sizeof(cl_uint), &max_constant_args, NULL);
        CHECK_OPENCL_ERROR(status, "clGetDeviceInfo failed");

		status = clGetDeviceInfo(deviceIds[i], CL_DEVICE_SINGLE_FP_CONFIG, sizeof(cl_device_fp_config), &single_fp_config, NULL);
        CHECK_OPENCL_ERROR(status, "clGetDeviceInfo failed");
	
		CL_DeviceInfo devInfo;
		devInfo.setDeviceInfo(deviceIds[i]);

        if(!b_quiet_mode) {
            printf("Device %d name: %s id: %p\n", i, deviceName, deviceIds[i]);
            printf("OpenCL version %s profile: %s\n", devInfo.openclCVersion, devInfo.profileType);
            printf("preferred Half width %d , native Half width: %d\n", devInfo.preferredHalfVecWidth, devInfo.nativeHalfVecWidth);
            printf("preferred Float width %d , native Float width: %d\n", devInfo.preferredFloatVecWidth, devInfo.nativeFloatVecWidth);
            printf("max read image args: %d\n", max_read_args);
            printf("max mem alloc size: %lu\n", max_mem_alloc);
            printf("max constant args: %d\n", max_constant_args);
            printf("device single FP config: FP_DENORM[%lu]\n", single_fp_config&CL_FP_DENORM);
            printf("image2d max width: %lu height: %lu\n", devInfo.image2dMaxWidth, devInfo.image2dMaxWidth);
            printf("max WG size: %lu\n", devInfo.maxWorkGroupSize);
            for(cl_uint i=0;i<devInfo.maxWorkItemDims;++i) {
                printf("max WI size[%d]: %lu\n", i, devInfo.maxWorkItemSizes[i]);
            }
            printf("supported extensions list: %s \n", devInfo.extensions);
        }
    }

    free(deviceIds);
    
    return true;
}

////////////////////////////////////////////////////////////////////////////////////
bool ocl_wait_for_event(cl_event *event, bool release /*= true*/)
{
    cl_int status = CL_SUCCESS;
    cl_int eventStatus = CL_QUEUED;
	while(eventStatus != CL_COMPLETE)
    {
        status = clGetEventInfo(
                        *event, 
                        CL_EVENT_COMMAND_EXECUTION_STATUS, 
                        sizeof(cl_int),
                        &eventStatus,
                        NULL);
		CHECK_OPENCL_ERROR(status, "clGetEventEventInfo Failed with Error Code:");
	}

	if(release)
	{
		status = clReleaseEvent(*event);
		CHECK_OPENCL_ERROR(status, "clReleaseEvent Failed with Error Code:");
	}

    return true;
}

////////////////////////////////////////////////////////////////////////////////////
bool ocl_get_event_status(cl_event event, cl_int* ev_status)
{
    assert(ev_status);
    *ev_status = CL_QUEUED;
    cl_int status = clGetEventInfo(
                        event, 
                        CL_EVENT_COMMAND_EXECUTION_STATUS, 
                        sizeof(cl_int),
                        ev_status,
                        NULL);
    CHECK_OPENCL_ERROR(status, "clGetEventEventInfo Failed with Error Code:");
    return true;
}

////////////////////////////////////////////////////////////////////////////////////
bool ocl_get_event_ref_count(cl_event event, cl_uint* ref_count)
{
    cl_int status = clGetEventInfo(event, CL_EVENT_REFERENCE_COUNT, sizeof(cl_uint), ref_count, NULL);
    CHECK_OPENCL_ERROR(status, "clGetEventProfilingInfo(COMMAND_START) failed");
    return true;
}


////////////////////////////////////////////////////////////////////////////////////
bool ocl_get_exec_time_in_ms(cl_event* event, float* time_ms)
{
    cl_int status;
    cl_ulong start = 0, end = 0;

    assert(time_ms);
    *time_ms = 0.0f;

    cl_int ev_status = CL_QUEUED;
    assert(ocl_get_event_status(*event, &ev_status) && ev_status==CL_COMPLETE);

    status = clGetEventProfilingInfo(*event, CL_PROFILING_COMMAND_START, sizeof start, &start, NULL);
    CHECK_OPENCL_ERROR(status, "clGetEventProfilingInfo(COMMAND_START) failed");

    status = clGetEventProfilingInfo(*event, CL_PROFILING_COMMAND_END, sizeof end, &end, NULL);
    CHECK_OPENCL_ERROR(status, "clGetEventProfilingInfo(COMMAND_END) failed");

    *time_ms = (float)(end - start) / 1e6f; /* Convert nanoseconds to msecs */
    //printf("Profiling: Total kernel time was %5.2f msecs.\n", total);

    return true;
}

////////////////////////////////////////////////////////////////////////////////////
bool ocl_execute_kernel_1d_sync(cl_kernel k, int blockSize, int gridSize, float* exec_time/*=0*/)
{
	cl_int status;
	size_t globalWorkSize[3] = {1,1,1};
    size_t localWorkSize[3] = {1, 1, 1};
	
    // Set local and global work group sizes
	globalWorkSize[0] = gridSize * blockSize;
	localWorkSize[0] = blockSize;

    // Execute kernel on given device
    cl_event  eventND[1];
	status = clEnqueueNDRangeKernel(ocl_get_queue(), k, 1, NULL, globalWorkSize, localWorkSize, 0, 0, eventND );
    CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel failed.");

    status = clFlush(ocl_get_queue());
    CHECK_OPENCL_ERROR(status, "clFlush() failed");

    status = ocl_wait_for_event(&eventND[0], false);
    if(false == status)
	{
		clReleaseEvent(eventND[0]);
		return false;
	}

	if(exec_time)
	{
		ocl_get_exec_time_in_ms(&eventND[0], exec_time);
	}

	clReleaseEvent(eventND[0]);

    status = clFinish(ocl_get_queue());
    CHECK_OPENCL_ERROR(status, "clFinish failed.");

	return true;
}

////////////////////////////////////////////////////////////////////////////////////
bool ocl_execute_kernel_1d(cl_kernel k, int blockSize, int gridSize, int num, cl_event* wait_for, cl_event* out_event)
{
	cl_int status;
	size_t globalWorkSize[3] = {1,1,1};
	size_t localWorkSize[3] = {1, 1, 1};

	globalWorkSize[0] = gridSize * blockSize;
	localWorkSize[0] = blockSize;

	status = clEnqueueNDRangeKernel(ocl_get_queue(), k, 1, NULL, globalWorkSize, localWorkSize, num, wait_for, out_event);
	CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel failed.");

	return true;
}

////////////////////////////////////////////////////////////////////////////////////
bool ocl_execute_kernel_2d_sync(cl_kernel k, int blockSizeX, int blockSizeY, int gridSizeX, int gridSizeY, float* exec_time/*=0*/)
{
	cl_int status;
	size_t globalWorkSize[3] = {1,1,1};
    size_t localWorkSize[3] = {1, 1, 1};
	
    // Set local and global work group sizes
	globalWorkSize[0] = gridSizeX * blockSizeX;
	globalWorkSize[1] = gridSizeY * blockSizeY;
	localWorkSize[0] = blockSizeX;
	localWorkSize[1] = blockSizeY;

    // Execute kernel on given device
    cl_event  eventND[1];
	status = clEnqueueNDRangeKernel(ocl_get_queue(), k, 2, NULL, globalWorkSize, localWorkSize, 0, 0, eventND );
    CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel failed.");

    status = clFlush(ocl_get_queue());
    CHECK_OPENCL_ERROR(status, "clFlush() failed");

    status = ocl_wait_for_event(&eventND[0], false);
    if(false == status)
		return false;

	if(exec_time)
	{
		ocl_get_exec_time_in_ms(&eventND[0], exec_time);
	}

	clReleaseEvent(eventND[0]);

    status = clFinish(ocl_get_queue());
    CHECK_OPENCL_ERROR(status, "clFinish failed.");

	return true;
}

////////////////////////////////////////////////////////////////////////////////////
bool ocl_execute_kernel_2d(cl_kernel k, int blockSizeX, int blockSizeY, int gridSizeX, int gridSizeY, int num_wait_for, cl_event* wait_for, cl_event* out_event)
{
	cl_int status;
	size_t globalWorkSize[3] = {1, 1, 1};
    size_t localWorkSize[3] = {1, 1, 1};
	
    // Set local and global work group sizes
	globalWorkSize[0] = gridSizeX * blockSizeX;
	globalWorkSize[1] = gridSizeY * blockSizeY;
	localWorkSize[0] = blockSizeX;
	localWorkSize[1] = blockSizeY;

	status = clEnqueueNDRangeKernel(ocl_get_queue(), k, 2, NULL, globalWorkSize, localWorkSize, num_wait_for, wait_for, out_event);
    CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel failed.");

    return true;
}

////////////////////////////////////////////////////////////////////////////////////
bool ocl_execute_kernel_3d_sync(cl_kernel k, int blockSizeX, int blockSizeY, int blockSizeZ, int gridSizeX, int gridSizeY, int gridSizeZ, float* exec_time/*=0*/)
{
	cl_int status;
	size_t globalWorkSize[3] = {1,1,1};
	size_t localWorkSize[3] = {1, 1, 1};

	// Set local and global work group sizes
	globalWorkSize[0] = gridSizeX * blockSizeX;
	globalWorkSize[1] = gridSizeY * blockSizeY;
	globalWorkSize[2] = gridSizeZ * blockSizeZ;
	localWorkSize[0] = blockSizeX;
	localWorkSize[1] = blockSizeY;
	localWorkSize[2] = blockSizeZ;

	// Execute kernel on given device
	cl_event  eventND[1];
	status = clEnqueueNDRangeKernel(ocl_get_queue(), k, 3, NULL, globalWorkSize, localWorkSize, 0, 0, eventND );
	CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel failed.");

	status = clFlush(ocl_get_queue());
	CHECK_OPENCL_ERROR(status, "clFlush() failed");

	status = ocl_wait_for_event(&eventND[0], false);
	if(false == status)
		return false;

	if(exec_time)
	{
		ocl_get_exec_time_in_ms(&eventND[0], exec_time);
	}

	clReleaseEvent(eventND[0]);

	status = clFinish(ocl_get_queue());
	CHECK_OPENCL_ERROR(status, "clFinish failed.");

	return true;
}

//===========================================================
char* ScopedMap::map(unsigned int flags, int size)
{
	cl_int status;
	assert(!was_mapped_);
	pdata_ = (char*)clEnqueueMapBuffer(ocl_get_queue(), buf_, CL_TRUE, flags, 0, size, 0, NULL, NULL, &status);
	CHECK_OPENCL_ERROR_R(status, "clEnqueueMapBuffer failed.", 0);
	was_mapped_ = true;
	return pdata_;
}

ScopedMap::~ScopedMap()
{
	if(was_mapped_)
	{
		clEnqueueUnmapMemObject(ocl_get_queue(), buf_, pdata_, 0, NULL, NULL);
	}

}
//==============================================================
