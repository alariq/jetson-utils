#ifndef __OCLBUFFER_H__
#define __OCLBUFFER_H__

/*
struct ocl_base_buffer {
	virtual ~ocl_base_buffer() = 0 {}
};
*/

template <typename T> struct ocl_buffer;

template <typename T>
bool ocl_create_buffer(
		ocl_buffer<T>* b,
		cl_context ctx, 
		size_t num_el, 
		cl_mem_flags flags, 
		void* host_ptr, 
		cl_int* errcode_ret);

template <typename T> 
struct ocl_buffer {

    template <typename S>
    friend bool ocl_create_buffer(
			    ocl_buffer<S>* b,
			    cl_context ctx, 
			    size_t num_el, 
			    cl_mem_flags flags, 
			    void* host_ptr, 
			    cl_int* errcode_ret);

	ocl_buffer():buf_(0), flags_(0), size_(0), host_ptr_(0), mapped_ptr_(0)//, num_mapped_(0)
	{}

	T* map(cl_command_queue queue, 
			bool blocking, 
			cl_map_flags flags, 
			size_t num_el,
			size_t offset = 0, 
			cl_uint num_events_in_wait_list = 0,
			const cl_event* event_wait_list = 0,
			cl_event* event = 0,
			cl_int* errcode_ret = 0)
	{
		assert(mapped_ptr_ == 0);

		cl_int errcode;
		mapped_ptr_ = (T*)clEnqueueMapBuffer(queue, buf_, blocking, flags, offset, sizeof(T)*num_el,
			       num_events_in_wait_list,
		       	       event_wait_list,
			       event,
			       &errcode);

		if(errcode_ret) *errcode_ret = errcode;
		CHECK_OPENCL_ERROR_R(errcode, "Failed to map buffer\n", 0);
		return mapped_ptr_;
	}

	void unmap(cl_command_queue queue, 
		cl_uint num_events_in_wait_list = 0,
		const cl_event* event_wait_list = 0,
		cl_event* event = 0)
	{
		assert(mapped_ptr_ /*&& num_mapped_*/);

		//if(!--num_mapped_)
		{
			cl_int status = clEnqueueUnmapMemObject(queue, buf_, mapped_ptr_, 
						num_events_in_wait_list, event_wait_list, event);
			CHECK_OPENCL_ERROR_R(status, "Failed to unmap buffer\n", );
			mapped_ptr_ = 0;
		}
	}

	const cl_mem& buf() { return buf_; }	
	size_t num_el() { return num_el_; }
	T* mapped_ptr() { return mapped_ptr_; }

	void release()
	{
		assert(0==mapped_ptr_);
		if(buf_)
			clReleaseMemObject(buf_);
	}

	~ocl_buffer<T>()
	{
	
	}

private:	
	cl_mem buf_;

	cl_mem_flags flags_;
	size_t size_;
	size_t num_el_;
	void* host_ptr_;

	T* mapped_ptr_;
	//size_t num_mapped_;
};


template <typename T>
bool ocl_create_buffer(
		ocl_buffer<T>* b,
		cl_context ctx, 
		size_t num_el, 
		cl_mem_flags flags, 
		void* host_ptr, 
		cl_int* errcode_ret)
{
	assert(b);
	cl_int errcode;
	b->flags_ = flags;
	b->size_ = num_el*sizeof(T);
	b->num_el_ = num_el;
	b->host_ptr_ = host_ptr;
	b->buf_ = clCreateBuffer(ctx, b->flags_, b->size_, b->host_ptr_, &errcode);
	if(errcode_ret) *errcode_ret = errcode;
	CHECK_OPENCL_ERROR(errcode, "Failed to create buffer\n");
	return true;
}


// slow CPU version
template <typename T>
void ocl_memset(ocl_buffer<T>* b,
		cl_command_queue queue,
		const T* value,
		size_t num_el,
		size_t offset = 0)
{
	//assert(b->flags_ & CL_MAP_WRITE) // eh no...
	T* p = b->map(queue, true, CL_MAP_WRITE, num_el);
	for(size_t i=0;i<num_el;++i)
		memcpy(p + i, value, sizeof(T));
	b->unmap(queue);
}
		




#endif // _OCLBUFFER_H__
