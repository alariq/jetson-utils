#ifndef __CL_UTILS_H__
#define __CL_UTILS_H__

#include <CL/cl.h>
#include <CL/cl_gl.h>
#include<CL/cl_ext.h>
#ifdef PLATFORM_WINDOWS
#include <windows.h>
#else
//#include <GL/glx.h>
#endif

#include <stdio.h>

//
//typedef CL_API_ENTRY cl_mem (CL_API_CALL *clCreateFromGLTexture_fn)(
//					  cl_context      /* context */,
//                      cl_mem_flags    /* flags */,
//                      cl_GLenum       /* target */,
//                      cl_GLint        /* miplevel */,
//                      cl_GLuint       /* texture */,
//                      cl_int *        /* errcode_ret */);
//
//#define clCreateFromGLTexture clCreateFromGLTexture_proc
//static clCreateFromGLTexture_fn clCreateFromGLTexture;

typedef CL_API_ENTRY cl_mem (CL_API_CALL *clImportMemoryARM_fn)(
		cl_context context,
		cl_mem_flags flags,
		const cl_import_properties_arm *properties,
		void *memory,
		size_t size,
		cl_int *errorcode_ret);

#define clImportMemoryARM clImportMemoryARM_proc
extern clImportMemoryARM_fn clImportMemoryARM;


struct oclInitParams {
#ifdef PLATFORM_WINDOWS
	HWND hwnd_;
	HGLRC glcontext_;
#else
	struct _XDisplay* display_;
	void* egl_display_;
	struct __GLXcontextRec *glcontext_;
#endif
	bool b_enable_interop;
	bool b_enable_import_memory_arm;
	cl_device_type device_type_;
};

bool ocl_init(const oclInitParams* params, cl_command_queue_properties queue_prop/* = 0*/);
bool ocl_create_context(const oclInitParams* params, cl_platform_id platform, cl_context &context, cl_device_id &interopDevice);
bool ocl_displayDevices(cl_platform_id platform, cl_device_type deviceType);
bool ocl_getDevices(cl_context &context, cl_device_id **devices, size_t* numDevices);
bool ocl_displayDevices(cl_platform_id platform, cl_device_type deviceType);
bool ocl_wait_for_event(cl_event *event, bool release = true);

bool ocl_get_exec_time_in_ms(cl_event* event, float* time_ms);

cl_context ocl_get_context(void);
cl_device_id* ocl_get_devices();
unsigned int ocl_get_device_id();
cl_command_queue ocl_get_queue();

//bool ocl_get_platform(cl_platform_id &platform, int platformId, bool platformIdEnabled);

template<typename T>
const char* ocl_get_error_code_str(T input)
{
    int errorCode = (int)input;
    switch(errorCode)
    {
        case CL_DEVICE_NOT_FOUND:
            return "CL_DEVICE_NOT_FOUND";
        case CL_DEVICE_NOT_AVAILABLE:
            return "CL_DEVICE_NOT_AVAILABLE";               
        case CL_COMPILER_NOT_AVAILABLE:
            return "CL_COMPILER_NOT_AVAILABLE";           
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
            return "CL_MEM_OBJECT_ALLOCATION_FAILURE";      
        case CL_OUT_OF_RESOURCES:
            return "CL_OUT_OF_RESOURCES";                    
        case CL_OUT_OF_HOST_MEMORY:
            return "CL_OUT_OF_HOST_MEMORY";                 
        case CL_PROFILING_INFO_NOT_AVAILABLE:
            return "CL_PROFILING_INFO_NOT_AVAILABLE";        
        case CL_MEM_COPY_OVERLAP:
            return "CL_MEM_COPY_OVERLAP";                    
        case CL_IMAGE_FORMAT_MISMATCH:
            return "CL_IMAGE_FORMAT_MISMATCH";               
        case CL_IMAGE_FORMAT_NOT_SUPPORTED:
            return "CL_IMAGE_FORMAT_NOT_SUPPORTED";         
        case CL_BUILD_PROGRAM_FAILURE:
            return "CL_BUILD_PROGRAM_FAILURE";              
        case CL_MAP_FAILURE:
            return "CL_MAP_FAILURE";                         
        case CL_MISALIGNED_SUB_BUFFER_OFFSET:
            return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
        case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
            return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
        case CL_INVALID_VALUE:
            return "CL_INVALID_VALUE";                      
        case CL_INVALID_DEVICE_TYPE:
            return "CL_INVALID_DEVICE_TYPE";               
        case CL_INVALID_PLATFORM:
            return "CL_INVALID_PLATFORM";                   
        case CL_INVALID_DEVICE:
            return "CL_INVALID_DEVICE";                    
        case CL_INVALID_CONTEXT:
            return "CL_INVALID_CONTEXT";                    
        case CL_INVALID_QUEUE_PROPERTIES:
            return "CL_INVALID_QUEUE_PROPERTIES";           
        case CL_INVALID_COMMAND_QUEUE:
            return "CL_INVALID_COMMAND_QUEUE";              
        case CL_INVALID_HOST_PTR:
            return "CL_INVALID_HOST_PTR";                   
        case CL_INVALID_MEM_OBJECT:
            return "CL_INVALID_MEM_OBJECT";                  
        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
            return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";    
        case CL_INVALID_IMAGE_SIZE:
             return "CL_INVALID_IMAGE_SIZE";                 
        case CL_INVALID_SAMPLER:
            return "CL_INVALID_SAMPLER";                    
        case CL_INVALID_BINARY:
            return "CL_INVALID_BINARY";                     
        case CL_INVALID_BUILD_OPTIONS:
            return "CL_INVALID_BUILD_OPTIONS";              
        case CL_INVALID_PROGRAM:
            return "CL_INVALID_PROGRAM";                    
        case CL_INVALID_PROGRAM_EXECUTABLE:
            return "CL_INVALID_PROGRAM_EXECUTABLE";          
        case CL_INVALID_KERNEL_NAME:
            return "CL_INVALID_KERNEL_NAME";                
        case CL_INVALID_KERNEL_DEFINITION:
            return "CL_INVALID_KERNEL_DEFINITION";          
        case CL_INVALID_KERNEL:
            return "CL_INVALID_KERNEL";                     
        case CL_INVALID_ARG_INDEX:
            return "CL_INVALID_ARG_INDEX";                   
        case CL_INVALID_ARG_VALUE:
            return "CL_INVALID_ARG_VALUE";                   
        case CL_INVALID_ARG_SIZE:
            return "CL_INVALID_ARG_SIZE";                    
        case CL_INVALID_KERNEL_ARGS:
            return "CL_INVALID_KERNEL_ARGS";                
        case CL_INVALID_WORK_DIMENSION:
            return "CL_INVALID_WORK_DIMENSION";              
        case CL_INVALID_WORK_GROUP_SIZE:
            return "CL_INVALID_WORK_GROUP_SIZE";             
        case CL_INVALID_WORK_ITEM_SIZE:
            return "CL_INVALID_WORK_ITEM_SIZE";             
        case CL_INVALID_GLOBAL_OFFSET:
            return "CL_INVALID_GLOBAL_OFFSET";              
        case CL_INVALID_EVENT_WAIT_LIST:
            return "CL_INVALID_EVENT_WAIT_LIST";             
        case CL_INVALID_EVENT:
            return "CL_INVALID_EVENT";                      
        case CL_INVALID_OPERATION:
            return "CL_INVALID_OPERATION";                 
        case CL_INVALID_GL_OBJECT:
            return "CL_INVALID_GL_OBJECT";                  
        case CL_INVALID_BUFFER_SIZE:
            return "CL_INVALID_BUFFER_SIZE";                 
        case CL_INVALID_MIP_LEVEL:
            return "CL_INVALID_MIP_LEVEL";                   
        case CL_INVALID_GLOBAL_WORK_SIZE:
            return "CL_INVALID_GLOBAL_WORK_SIZE";            
#ifdef CL_VERSION_1_1
	   case CL_INVALID_PROPERTY: return "CL_INVALID_PROPERTY";
#endif
#ifdef CL_VERSION_1_2
	   case CL_INVALID_IMAGE_DESCRIPTOR: return "CL_INVALID_IMAGE_DESCRIPTOR";
	   case CL_INVALID_COMPILER_OPTIONS: return "CL_INVALID_COMPILER_OPTIONS";
	   case CL_INVALID_LINKER_OPTIONS: return "CL_INVALID_LINKER_OPTIONS";
	   case CL_INVALID_DEVICE_PARTITION_COUNT: return "CL_INVALID_DEVICE_PARTITION_COUNT";
#endif
#ifdef CL_VERSION_2_0
	   case CL_INVALID_PIPE_SIZE: return "CL_INVALID_PIPE_SIZE";
	   case CL_INVALID_DEVICE_QUEUE: return "CL_INVALID_DEVICE_QUEUE";
#endif
#ifdef CL_VERSION_2_2
	   case CL_INVALID_SPEC_ID: return "CL_INVALID_SPEC_ID";
	   case CL_MAX_SIZE_RESTRICTION_EXCEEDED: return "CL_MAX_SIZE_RESTRICTION_EXCEEDED";
#endif
        //case CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR:
        //    return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
       // case CL_PLATFORM_NOT_FOUND_KHR:
        //    return "CL_PLATFORM_NOT_FOUND_KHR";
        //case CL_INVALID_PROPERTY_EXT:
        //    return "CL_INVALID_PROPERTY_EXT";
        //case CL_DEVICE_PARTITION_FAILED_EXT:
        //    return "CL_DEVICE_PARTITION_FAILED_EXT";
        //case CL_INVALID_PARTITION_COUNT_EXT:
        //    return "CL_INVALID_PARTITION_COUNT_EXT"; 
        default:
            return "";
    }

    return "";
}


template<typename T>
static int ocl_check_val(T input, T reference, const char* message)
{
	if(input==reference)
	{
		return true;
	}
	else
	{
		printf("OpenCL Error: %s Error code: %s (%d)\n", message, ocl_get_error_code_str(input), input);
		return false;
	}
}

static inline cl_int ocl_check_val2(cl_int rv, const char* message, const char* file, int line)
{
	if(rv != CL_SUCCESS)
	{
		fprintf(stderr, "%s:%d OpenCL Error: %s Error code: %s (%d)\n", file, line, message, ocl_get_error_code_str(rv), rv);
	}
	return rv;
}

#define oclCheckError(x, msg, file, line)  ocl_check_val2((x), msg, file, line) 

#define CHECK_OPENCL_ERROR(actual, msg) \
	if(false == ocl_check_val(actual, CL_SUCCESS, msg)) \
{ \
	printf("Location : %s : %d\n", __FILE__ , __LINE__); \
	return false; \
}

#define CHECK_OPENCL_ERROR_NORET(actual, msg) \
	if(false == ocl_check_val(actual, CL_SUCCESS, msg)) \
{ \
	printf("Location : %s : %d\n", __FILE__ , __LINE__); \
}


#define CHECK_OPENCL_ERROR_R(actual, msg, rv) \
	if(false == ocl_check_val(actual, CL_SUCCESS, msg)) \
{ \
	printf("Location : %s : %d\n", __FILE__ , __LINE__); \
	return rv; \
}


struct ScopedMap {
	cl_mem buf_;
	char* pdata_;
	int was_mapped_;
	ScopedMap(cl_mem buffer):buf_(buffer), pdata_(0), was_mapped_(false) {}
	~ScopedMap();
	char* map(unsigned int flags, int size);

	private:
		ScopedMap(const ScopedMap&);
		ScopedMap operator=(const ScopedMap&);
};

bool ocl_execute_kernel_1d(cl_kernel k, int blockSize, int gridSize, int num_wait_for, cl_event* wait_for, cl_event* out_event);
bool ocl_execute_kernel_1d_sync(cl_kernel k, int blockSize, int gridSize, float* exec_time/*=0*/);
bool ocl_execute_kernel_2d(cl_kernel k, int blockSizeX, int blockSizeY, int gridSizeX, int gridSizeY, int num_wait_for, cl_event* wait_for, cl_event* out_event);
bool ocl_execute_kernel_2d_sync(cl_kernel k, int blockSizeX, int blockSizeY, int gridSizeX, int gridSizeY, float* exec_time/*=0*/);
bool ocl_execute_kernel_3d_sync(cl_kernel k, int blockSizeX, int blockSizeY, int blockSizeZ, int gridSizeX, int gridSizeY, int gridSizeZ, float* exec_time/*=0*/);

bool ocl_get_event_status(cl_event event, cl_int* ev_status);
bool ocl_get_event_ref_count(cl_event event, cl_uint* ref_count);

#endif // __CL_UTILS_H__
