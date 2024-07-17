#include <CL/cl.h>
#include <cstdio>
#include <cstring>
#include "ocl_device.h" 
#include "ocl_utils.h" 

// Set all information for a given device id
bool CL_DeviceInfo::setDeviceInfo(cl_device_id deviceId)
{
    cl_int status = CL_SUCCESS;


    //Get device type
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_TYPE, 
                    sizeof(cl_device_type), 
                    &dType, 
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_TYPE) failed");

    //Get vender ID
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_VENDOR_ID, 
                    sizeof(cl_uint), 
                    &vendorId, 
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_VENDOR_ID) failed");

    //Get max compute units
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_MAX_COMPUTE_UNITS, 
                    sizeof(cl_uint), 
                    &maxComputeUnits, 
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_MAX_COMPUTE_UNITS) failed");

    //Get max work item dimensions
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS,
                    sizeof(cl_uint),
                    &maxWorkItemDims,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS) failed");

    //Get max work item sizes
    delete maxWorkItemSizes;
    maxWorkItemSizes = new size_t[maxWorkItemDims];

    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_MAX_WORK_ITEM_SIZES,
                    maxWorkItemDims * sizeof(size_t),
                    maxWorkItemSizes,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS) failed");

    // Maximum work group size
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_MAX_WORK_GROUP_SIZE,
                    sizeof(size_t),
                    &maxWorkGroupSize,
                    NULL);
   CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_MAX_WORK_GROUP_SIZE) failed");

    // Preferred vector sizes of all data types
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR,
                    sizeof(cl_uint),
                    &preferredCharVecWidth,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR) failed");

    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT,
                    sizeof(cl_uint),
                    &preferredShortVecWidth,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT) failed");

    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT,
                    sizeof(cl_uint),
                    &preferredIntVecWidth,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT) failed");

    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG,
                    sizeof(cl_uint),
                    &preferredLongVecWidth,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG) failed");

    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT,
                    sizeof(cl_uint),
                    &preferredFloatVecWidth,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT) failed");

    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE,
                    sizeof(cl_uint),
                    &preferredDoubleVecWidth,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE) failed");

    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF,
                    sizeof(cl_uint),
                    &preferredHalfVecWidth,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF) failed");

    // Clock frequency
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_MAX_CLOCK_FREQUENCY,
                    sizeof(cl_uint),
                    &maxClockFrequency,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_MAX_CLOCK_FREQUENCY) failed");

    // Address bits
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_ADDRESS_BITS,
                    sizeof(cl_uint),
                    &addressBits,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_ADDRESS_BITS) failed");

    // Maximum memory alloc size
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_MAX_MEM_ALLOC_SIZE,
                    sizeof(cl_ulong),
                    &maxMemAllocSize,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_MAX_MEM_ALLOC_SIZE) failed");

    // Image support
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_IMAGE_SUPPORT,
                    sizeof(cl_bool),
                    &imageSupport,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_IMAGE_SUPPORT) failed");

    // Maximum read image arguments
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_MAX_READ_IMAGE_ARGS,
                    sizeof(cl_uint),
                    &maxReadImageArgs,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_MAX_READ_IMAGE_ARGS) failed");

    // Maximum write image arguments
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_MAX_WRITE_IMAGE_ARGS,
                    sizeof(cl_uint),
                    &maxWriteImageArgs,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_MAX_WRITE_IMAGE_ARGS) failed");

    // 2D image and 3D dimensions
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_IMAGE2D_MAX_WIDTH,
                    sizeof(size_t),
                    &image2dMaxWidth,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_IMAGE2D_MAX_WIDTH) failed");

    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_IMAGE2D_MAX_HEIGHT,
                    sizeof(size_t),
                    &image2dMaxHeight,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_IMAGE2D_MAX_HEIGHT) failed");

    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_IMAGE3D_MAX_WIDTH,
                    sizeof(size_t),
                    &image3dMaxWidth,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_IMAGE3D_MAX_WIDTH) failed");

    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_IMAGE3D_MAX_HEIGHT,
                    sizeof(size_t),
                    &image3dMaxHeight,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_IMAGE3D_MAX_HEIGHT) failed");

    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_IMAGE3D_MAX_DEPTH,
                    sizeof(size_t),
                    &image3dMaxDepth,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_IMAGE3D_MAX_DEPTH) failed");

    // Maximum samplers
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_MAX_SAMPLERS,
                    sizeof(cl_uint),
                    &maxSamplers,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_MAX_SAMPLERS) failed");

    // Maximum parameter size
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_MAX_PARAMETER_SIZE,
                    sizeof(size_t),
                    &maxParameterSize,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_MAX_PARAMETER_SIZE) failed");

    // Memory base address align
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_MEM_BASE_ADDR_ALIGN,
                    sizeof(cl_uint),
                    &memBaseAddressAlign,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_MEM_BASE_ADDR_ALIGN) failed");

    // Minimum data type align size
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE,
                    sizeof(cl_uint),
                    &minDataTypeAlignSize,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE) failed");

    // Single precision floating point configuration
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_SINGLE_FP_CONFIG,
                    sizeof(cl_device_fp_config),
                    &singleFpConfig,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_SINGLE_FP_CONFIG) failed");
#ifndef NVIDIA_OCL_SDK
    // Double precision floating point configuration
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_DOUBLE_FP_CONFIG,
                    sizeof(cl_device_fp_config),
                    &doubleFpConfig,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_DOUBLE_FP_CONFIG) failed");
#endif
    // Global memory cache type
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_GLOBAL_MEM_CACHE_TYPE,
                    sizeof(cl_device_mem_cache_type),
                    &globleMemCacheType,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_GLOBAL_MEM_CACHE_TYPE) failed");

    // Global memory cache line size
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE,
                    sizeof(cl_uint),
                    &globalMemCachelineSize,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE) failed");

    // Global memory cache size
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_GLOBAL_MEM_CACHE_SIZE,
                    sizeof(cl_ulong),
                    &globalMemCacheSize,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_GLOBAL_MEM_CACHE_SIZE) failed");

    // Global memory size
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_GLOBAL_MEM_SIZE,
                    sizeof(cl_ulong),
                    &globalMemSize,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_GLOBAL_MEM_SIZE) failed");

    // Maximum constant buffer size
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE,
                    sizeof(cl_ulong),
                    &maxConstBufSize,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE) failed");

    // Maximum constant arguments
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_MAX_CONSTANT_ARGS,
                    sizeof(cl_uint),
                    &maxConstArgs,
                    NULL);
   CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_MAX_CONSTANT_ARGS) failed");

    // Local memory type
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_LOCAL_MEM_TYPE,
                    sizeof(cl_device_local_mem_type),
                    &localMemType,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_LOCAL_MEM_TYPE) failed");

    // Local memory size
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_LOCAL_MEM_SIZE,
                    sizeof(cl_ulong),
                    &localMemSize,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_LOCAL_MEM_SIZE) failed");

    // Error correction support
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_ERROR_CORRECTION_SUPPORT,
                    sizeof(cl_bool),
                    &errCorrectionSupport,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_ERROR_CORRECTION_SUPPORT) failed");

    // Profiling timer resolution
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_PROFILING_TIMER_RESOLUTION,
                    sizeof(size_t),
                    &timerResolution,
                    NULL);
   CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_PROFILING_TIMER_RESOLUTION) failed");

    // Endian little
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_ENDIAN_LITTLE,
                    sizeof(cl_bool),
                    &endianLittle,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_ENDIAN_LITTLE) failed");

    // Device available
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_AVAILABLE,
                    sizeof(cl_bool),
                    &available,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_AVAILABLE) failed");

    // Device compiler available
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_COMPILER_AVAILABLE,
                    sizeof(cl_bool),
                    &compilerAvailable,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_COMPILER_AVAILABLE) failed");

    // Device execution capabilities
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_EXECUTION_CAPABILITIES,
                    sizeof(cl_device_exec_capabilities),
                    &execCapabilities,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_EXECUTION_CAPABILITIES) failed");

    // Device queue properities
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_QUEUE_PROPERTIES,
                    sizeof(cl_command_queue_properties),
                    &queueProperties,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_QUEUE_PROPERTIES) failed");

    // Platform
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_PLATFORM,
                    sizeof(cl_platform_id),
                    &platform,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_PLATFORM) failed");

    // Device name
    size_t tempSize = 0;
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_NAME,
                    0,
                    NULL,
                    &tempSize);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_NAME) failed");

	delete name;
    name = new char[tempSize];

    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_NAME,
                    sizeof(char) * tempSize,
                    name,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_NAME) failed");

    // Vender name
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_VENDOR,
                    0,
                    NULL,
                    &tempSize);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_VENDOR) failed");

	delete vendorName;
    vendorName = new char[tempSize];

    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_VENDOR,
                    sizeof(char) * tempSize,
                    vendorName,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_VENDOR) failed");

    // Driver name
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DRIVER_VERSION,
                    0,
                    NULL,
                    &tempSize);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DRIVER_VERSION) failed");

	delete driverVersion;
    driverVersion = new char[tempSize];

    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DRIVER_VERSION,
                    sizeof(char) * tempSize,
                    driverVersion,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DRIVER_VERSION) failed");

    // Device profile
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_PROFILE,
                    0,
                    NULL,
                    &tempSize);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_PROFILE) failed");
    delete profileType;
    profileType = new char[tempSize];

    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_PROFILE,
                    sizeof(char) * tempSize,
                    profileType,
                    NULL);
   CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_PROFILE) failed");

    // Device version
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_VERSION,
                    0,
                    NULL,
                    &tempSize);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_VERSION) failed");

	delete deviceVersion;
    deviceVersion = new char[tempSize];

    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_VERSION,
                    sizeof(char) * tempSize,
                    deviceVersion,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_VERSION) failed");

    // Device extensions
    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_EXTENSIONS,
                    0,
                    NULL,
                    &tempSize);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_EXTENSIONS) failed");

	delete extensions;
    extensions = new char[tempSize];

    status = clGetDeviceInfo(
                    deviceId, 
                    CL_DEVICE_EXTENSIONS,
                    sizeof(char) * tempSize,
                    extensions,
                    NULL);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_EXTENSIONS) failed");

    // Device parameters of OpenCL 1.1 Specification
#ifdef CL_VERSION_1_1
    char* vStart = strchr(deviceVersion, ' ');
    if(strncmp(vStart+1, "1.0", 3) > 0)
    {
        // Native vector sizes of all data types
        status = clGetDeviceInfo(
                        deviceId, 
                        CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR,
                        sizeof(cl_uint),
                        &nativeCharVecWidth,
                        NULL);
        CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR) failed");

        status = clGetDeviceInfo(
                        deviceId, 
                        CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT,
                        sizeof(cl_uint),
                        &nativeShortVecWidth,
                        NULL);
        CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT) failed");

        status = clGetDeviceInfo(
                        deviceId, 
                        CL_DEVICE_NATIVE_VECTOR_WIDTH_INT,
                        sizeof(cl_uint),
                        &nativeIntVecWidth,
                        NULL);
        CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_NATIVE_VECTOR_WIDTH_INT) failed");

        status = clGetDeviceInfo(
                        deviceId, 
                        CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG,
                        sizeof(cl_uint),
                        &nativeLongVecWidth,
                        NULL);
        CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG) failed");

        status = clGetDeviceInfo(
                        deviceId, 
                        CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT,
                        sizeof(cl_uint),
                        &nativeFloatVecWidth,
                        NULL);
        CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT) failed");

        status = clGetDeviceInfo(
                        deviceId, 
                        CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE,
                        sizeof(cl_uint),
                        &nativeDoubleVecWidth,
                        NULL);
        CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE) failed");

        status = clGetDeviceInfo(
                        deviceId, 
                        CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF,
                        sizeof(cl_uint),
                        &nativeHalfVecWidth,
                        NULL);
        CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF) failed");

        // Host unified memory
        status = clGetDeviceInfo(
                        deviceId, 
                        CL_DEVICE_HOST_UNIFIED_MEMORY,
                        sizeof(cl_bool),
                        &hostUnifiedMem,
                        NULL);
        CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_HOST_UNIFIED_MEMORY) failed");

        // Device OpenCL C version
        status = clGetDeviceInfo(
                        deviceId, 
                        CL_DEVICE_OPENCL_C_VERSION,
                        0,
                        NULL,
                        &tempSize);
        CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_OPENCL_C_VERSION) failed");

		delete openclCVersion;
        openclCVersion = new char[tempSize];

        status = clGetDeviceInfo(
                        deviceId, 
                        CL_DEVICE_OPENCL_C_VERSION,
                        sizeof(char) * tempSize,
                        openclCVersion,
                        NULL);
        CHECK_OPENCL_ERROR(status, "clGetDeviceIDs(CL_DEVICE_OPENCL_C_VERSION) failed");
    }
#endif

    return true;
}


