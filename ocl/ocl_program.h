#ifndef __CL_PROGRAM_H__
#define __CL_PROGRAM_H__

#include<map>
#include<string>

struct ocl_program {

	ocl_program():program_(0), context_(0),devices_(0), num_devices_(0) {}
	bool load(const char* fname, const char* flags, cl_context ctx, cl_device_id* devices, int num_devices);
	bool load_binary(const char* flags, cl_context ctx, cl_device_id* devices, int num_devices, size_t* sizes, const unsigned char** binaries);
	bool create_kernel(const char* kname);
	cl_kernel get_kernel_by_name(const char* kname);
	bool get_binary(unsigned char**& pout, size_t** sizes, int* num);
	bool is_valid() { return 0 != program_; }
	bool reload();
    void destroy();

	cl_program program_;
	cl_context context_;
	std::string fname_;
	std::string flags_;
	cl_device_id* devices_;
	int num_devices_;

	typedef std::map<std::string, cl_kernel> Kernels_t;
	Kernels_t kernels_;

};

#endif // __CL_PROGRAM_H__
