/*
 * Copyright (c) 2018, NVIDIA CORPORATION. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef __MULTITHREAD_RINGBUFFER_INLINE_H_
#define __MULTITHREAD_RINGBUFFER_INLINE_H_

#if WITH_CUDA
#include "cudaMappedMemory.h"
#endif
#if WITH_OPENCL
#include <CL/cl.h>
#include "ocl_utils.h"
#include "oclUtility.h"
#endif
#include <assert.h>
#include "logging.h"


// constructor
RingBuffer::RingBuffer( uint32_t flags )
{
	mFlags = flags;
	mBuffers = NULL;
	mBufferSize = 0;
	mNumBuffers = 0;
	mReadOnce = false;
	mLatestRead = 0;
	mLatestWrite = 0;
}


// destructor
RingBuffer::~RingBuffer()
{
	Free();
	
	if( mBuffers != NULL )
	{
		free(mBuffers);
		mBuffers = NULL;
	}
}


// Alloc
inline bool RingBuffer::Alloc( uint32_t numBuffers, size_t size, uint32_t flags )
{
	if( numBuffers == mNumBuffers && size <= mBufferSize && (flags & ZeroCopy) == (mFlags & ZeroCopy) )
		return true;
	
	Free();
	
	if( mBuffers != NULL && mNumBuffers != numBuffers )
	{
		free(mBuffers);
		mBuffers = NULL;
		free(mMappedPtrs);
		mMappedPtrs = NULL;
	}
	
	if( mBuffers == NULL )
	{
		const size_t bufferListSize = numBuffers * sizeof(void*);
		mBuffers = (void**)malloc(bufferListSize);
		memset(mBuffers, 0, bufferListSize);
		mMappedPtrs = (void**)malloc(bufferListSize);
		memset(mMappedPtrs, 0, bufferListSize);
	}
	
	for( uint32_t n=0; n < numBuffers; n++ )
	{
#if WITH_CUDA
		if( flags & ZeroCopy )
		{
			if( !cudaAllocMapped(&mBuffers[n], size) )
			{
				LogError(LOG_CUDA "RingBuffer -- failed to allocate zero-copy buffer of %zu bytes\n", size);
				return false;
			}
		}
		else
		{
			if( CUDA_FAILED(cudaMalloc(&mBuffers[n], size)) )
			{
				LogError(LOG_CUDA "RingBuffer -- failed to allocate CUDA buffer of %zu bytes\n", size);
				return false;
			}

		}
#elif WITH_OPENCL
		cl_mem_flags mem_flags = CL_MEM_READ_WRITE;
		if(flags & ZeroCopy) {
			mem_flags |= CL_MEM_ALLOC_HOST_PTR;
		}
		cl_int status;
		mBuffers[n] = clCreateBuffer(ocl_get_context(), mem_flags, size, nullptr, &status);
		if( OCL_FAILED(status) )
		{
			LogError(LOG_OCL "RingBuffer -- failed to allocate OpenCL buffer of %zu bytes\n", size);
			return false;
		}
#else
		mBuffers[n] = malloc(size);
		asdfa
#endif
		mMappedPtrs[n] = nullptr;
	}
		
	LogVerbose(LOG_CUDA "allocated %u ring buffers (%zu bytes each, %zu bytes total)\n", numBuffers, size, size * numBuffers);
	
	mNumBuffers = numBuffers;
	mBufferSize = size;
	mFlags     |= flags;
	
	return true;
}


// Free
inline void RingBuffer::Free()
{
	if( !mBuffers || mNumBuffers == 0 )
		return;
	
	for( uint32_t n=0; n < mNumBuffers; n++ )
	{
#if WITH_CUDA
		if( mFlags & ZeroCopy )
			CUDA(cudaFreeHost(mBuffers[n]));
		else
			CUDA(cudaFree(mBuffers[n]));
#elif WITH_OPENCL
		clReleaseMemObject((cl_mem)mBuffers[n]);
#else
		free(mBuffers[n]);
#endif		
		mBuffers[n] = NULL;
		assert(mMappedPtrs[n]  == nullptr);
	}
}


// Peek
inline void* RingBuffer::Peek( uint32_t flags, bool b_map )
{
	flags |= mFlags;

	if( !mBuffers || mNumBuffers == 0 )
	{
		LogError(LOG_CUDA "RingBuffer::Peek() -- error, must call RingBuffer::Alloc() first\n");
		return NULL;
	}

	if( flags & Threaded )
		mMutex.Lock();

	int bufferIndex = -1;

	if( flags & Write )
		bufferIndex = (mLatestWrite + 1) % mNumBuffers;
	else if( flags & ReadLatest )
		bufferIndex = mLatestWrite;
	else if( flags & Read )
		bufferIndex = mLatestRead;
	
	if( flags & Threaded )
		mMutex.Unlock();

	if( bufferIndex < 0 )
	{
		LogError(LOG_CUDA "RingBuffer::Peek() -- error, invalid flags (must be Write or Read flags)\n");
		return NULL;
	}

#if WITH_OPENCL
	if(b_map) {
		if(mMappedPtrs[bufferIndex] != nullptr) {
			LogWarning("Peek: Buffer %d was mapped, and now want to map again, not mapping twice\n", bufferIndex);
		} else {
			cl_int status;
			cl_int ocl_flags = (flags & RingBuffer::Write) ? CL_MAP_WRITE : 0;
			ocl_flags |= (flags & RingBuffer::Read) ? CL_MAP_READ : 0;
			assert(mMappedPtrs[bufferIndex] == nullptr);
			mMappedPtrs[bufferIndex] = clEnqueueMapBuffer(ocl_get_queue(), (cl_mem)mBuffers[bufferIndex], CL_TRUE, ocl_flags, 0, mBufferSize, 0, NULL, NULL,&status);
			CHECK_OPENCL_ERROR_NORET(status, "clEnqueueMapBuffer");
			LogDebug("Peek: Buffer[%p] %d was mapped with flags: %d\n", mBuffers[bufferIndex], bufferIndex, ocl_flags);
		}
		return mMappedPtrs[bufferIndex];

	} else {
		LogWarning("Peek: Buffer %d was mapped, so unmapping\n", bufferIndex);
		if(mMappedPtrs[bufferIndex] != nullptr) {
			Unmap(mMappedPtrs[bufferIndex]);
		}
		return mBuffers[bufferIndex];
	}
#else 
	return mBuffers[bufferIndex];
#endif
}


// Next
inline void* RingBuffer::Next( uint32_t flags, bool b_map)
{
	flags |= mFlags;

	if( !mBuffers || mNumBuffers == 0 )
	{
		LogError(LOG_CUDA "RingBuffer::Next() -- error, must call RingBuffer::Alloc() first\n");
		return NULL;
	}

	if( flags & Threaded )
		mMutex.Lock();

	int bufferIndex = -1;

	if( flags & Write )
	{
		mLatestWrite = (mLatestWrite + 1) % mNumBuffers;
		bufferIndex  = mLatestWrite;
		mReadOnce    = false;
	}
	else if( (flags & ReadOnce) && mReadOnce )
	{
		if( flags & Threaded )
			mMutex.Unlock();

		if(mMappedPtrs[bufferIndex] != nullptr) {
			LogWarning("Next: Buffer[%p] %d was mapped, and now want to map again, not mapping twice\n", mBuffers[bufferIndex], bufferIndex);
		}

		LogDebug("Got buffer NULL\n");
		return NULL;
	}
	else if( flags & ReadLatest )
	{
		mLatestRead = mLatestWrite;
		bufferIndex = mLatestWrite;
		LogDebug("Got buffer: %d\n", bufferIndex);
		if(mMappedPtrs[bufferIndex] != nullptr) {
			LogWarning("Next: Buffer[%p] %d was mapped, and now want to map again, not mapping twice\n", mBuffers[bufferIndex], bufferIndex);
		}

		mReadOnce   = true;
	}
	else if( flags & Read )
	{
		mLatestRead = (mLatestRead + 1) % mNumBuffers;
		bufferIndex = mLatestRead;
		mReadOnce   = true;
	}
	
	if( flags & Threaded )
		mMutex.Unlock();

	if( bufferIndex < 0 )
	{
		LogError(LOG_CUDA "RingBuffer::Next() -- error, invalid flags (must be Write or Read flags)\n");
		return NULL;
	}

#if WITH_OPENCL
	if(b_map) {
		if(mMappedPtrs[bufferIndex] != nullptr) {
			LogWarning("Next: Buffer[%p] %d was mapped, and now want to map again, not mapping twice\n", mBuffers[bufferIndex], bufferIndex);
		} else {
			cl_int status;
			cl_int ocl_flags = (flags & RingBuffer::Write) ? CL_MAP_WRITE : 0;
			ocl_flags |= (flags & RingBuffer::Read) ? CL_MAP_READ : 0;
			assert(mMappedPtrs[bufferIndex] == nullptr);
			mMappedPtrs[bufferIndex] = clEnqueueMapBuffer(ocl_get_queue(), (cl_mem)mBuffers[bufferIndex], CL_TRUE, ocl_flags, 0, mBufferSize, 0, NULL, NULL,&status);
			CHECK_OPENCL_ERROR_NORET(status, "clEnqueueMapBuffer");
			LogDebug("Next: Buffer[%p] %d was mapped with flags: %d\n", mBuffers[bufferIndex], bufferIndex, ocl_flags);
		}
		return mMappedPtrs[bufferIndex];
	} else {
		if(mMappedPtrs[bufferIndex] != nullptr) {
			LogWarning("Next Buffer[%p] %d was mapped, so unmapping\n", mBuffers[bufferIndex], bufferIndex);
			Unmap(mMappedPtrs[bufferIndex]);
		}
		return mBuffers[bufferIndex];
	}
#else 
	return mBuffers[bufferIndex];
#endif
}


inline void RingBuffer::Unmap(void* mapped_ptr) {
#if WITH_OPENCL
	if(mapped_ptr == nullptr) {
		LogWarning("Unmap: Buffer with nullptr\n");
	}
	bool b_found = false;
	for(int i = 0; i< mNumBuffers; ++i) {
		if(mapped_ptr == mMappedPtrs[i]) {
			OCL(clEnqueueUnmapMemObject(ocl_get_queue(), (cl_mem)mBuffers[i], mapped_ptr, 0, nullptr, nullptr));
			mMappedPtrs[i] = nullptr;
			b_found = true;
			LogDebug("Unmap: Buffer[%p] %d was Unmapped\n", mBuffers[i], i);

#define DUMP_IMAGES 0
#if DUMP_IMAGES
			cl_int status;
			void* mp = clEnqueueMapBuffer(ocl_get_queue(), (cl_mem)mBuffers[i], CL_TRUE, CL_MAP_READ, 0, mBufferSize, 0, NULL, NULL,&status);
			CHECK_OPENCL_ERROR_NORET(status, "clEnqueueMapBuffer");
			int static counter = 0;
			char fname[256];
			sprintf(&fname[0], "dump/dump%d_yuv2.img", counter++);
			if(FILE* f = fopen(fname, "wb")) {
				fwrite(mp, mBufferSize, 1, f); 
				fclose(f);
			}
			OCL(clEnqueueUnmapMemObject(ocl_get_queue(), (cl_mem)mBuffers[i], mp, 0, nullptr, nullptr));
#endif
			break;
		}
	}
	if(!b_found) {
		LogError("Unmap: did not found mapped buffer!\n");
	}
	assert(b_found); (void)b_found;
#endif
}


// GetFlags
inline uint32_t RingBuffer::GetFlags() const
{
	return mFlags;
}
	

// SetFlags
inline void RingBuffer::SetFlags( uint32_t flags )
{
	mFlags = flags;
}
	

// SetThreaded
inline void RingBuffer::SetThreaded( bool threaded )
{
	if( threaded )
		mFlags |= Threaded;
	else
		mFlags &= ~Threaded;
}


#endif
