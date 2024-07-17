#include <string>
#include <cassert>
#include "ocl_utils.h"
#include "ocl_program.h"
#include "coclinit.h"

int cocl_init(void)
{
	return ocl_init(0,0)==false ? 0 : 1;
}

int cocl_load_program(cocl_program* program, const char* path, const char* opt)
{
	assert(program);
	ocl_program* pr = new ocl_program();
	if(false == pr->load(path, opt, ocl_get_context(), ocl_get_devices(), 1))
	{
		delete pr;
		return 0;
	}
	*program = pr;
	return 1;
}

int cocl_create_kernel(cocl_program program, const char* kname)
{
	assert(program);
	ocl_program* pr = (ocl_program*)program;
	if(pr->create_kernel(kname))
	{
		return 1;
	}
	return 0;
}

cocl_kernel cocl_program_get_kernel(cocl_program program, const char* kname)
{
	ocl_program* pr = (ocl_program*)program;
	if(pr->kernels_.count(kname))
		return pr->kernels_[kname];
	return 0;
}

cocl_context cocl_get_context()
{
	return ocl_get_context();
}

int cocl_wait_for_event_and_release(cocl_event *event)
{
	return ocl_wait_for_event(event) == true ? 1 : 0;
}

int cocl_wait_for_event(cocl_event *event, int release)
{
	return ocl_wait_for_event(event, release!=0) == true ? 1 : 0;
}

cocl_queue cocl_get_queue()
{
	return ocl_get_queue();
}

int cocl_check_error(cl_int status, const char* msg)
{
	return ocl_check_val(status, CL_SUCCESS, msg)!=0 ? 0 : 1;
}

int cocl_get_exec_time_in_ms(cl_event* event, float* time_ms)
{
	return ocl_get_exec_time_in_ms(event, time_ms)==true ? 1 : 0;
}
