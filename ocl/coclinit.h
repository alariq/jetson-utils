#ifndef __OCL_JPEG_H__
#define __OCL_JPEG_H__

#include <CL/cl.h>

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#endif

struct ocl_program;

typedef ocl_program* cocl_program;
typedef cl_mem cocl_buf;
typedef cl_context cocl_context;
typedef cl_event cocl_event;
typedef cl_kernel cocl_kernel;
typedef cl_command_queue cocl_queue;

#ifdef __cplusplus
//#ifndef DONT_USE_EXTERN_C
extern "C" {
#endif
//#endif

int cocl_init(void);
int cocl_load_program(cocl_program* program, const char* path, const char* opt);
int cocl_create_kernel(cocl_program program, const char* kname);
cocl_kernel cocl_program_get_kernel(cocl_program program, const char* kname);
int cocl_wait_for_event_and_release(cocl_event *event);
int cocl_wait_for_event(cocl_event *event, int release);
int cocl_check_error(cl_int status, const char* msg);
int cocl_get_exec_time_in_ms(cl_event* event, float* time_ms);

cocl_context cocl_get_context();
cocl_queue cocl_get_queue();

//cocl_buf	cocl_create_buffer();

#define CCHECK_OPENCL_ERROR(actual, msg) \
if(TRUE == cocl_check_error(actual, msg)) \
{ \
	printf("Location : %s : %d\n", __FILE__ , __LINE__); \
	return FALSE; \
}


#define CCHECK_OPENCL_ERROR_NORET(actual, msg) \
	if(TRUE == cocl_check_error(actual, msg)) \
{ \
	printf("Location : %s : %d\n", __FILE__ , __LINE__); \
}

#ifdef __cplusplus
//#ifndef DONT_USE_EXTERN_C
}
#endif
//#endif

#endif // __OCL_JPEG_H__
