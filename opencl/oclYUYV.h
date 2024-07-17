#pragma once

#include "oclUtility.h"
#include "ocl_program.h"


//////////////////////////////////////////////////////////////////////////////////
/// @name YUV YV12 4:2:0 planar to RGB
/// @see cudaConvertColor() from cudaColorspace.h for automated format conversion
/// @ingroup colorspace
//////////////////////////////////////////////////////////////////////////////////

///@{

/**
 * Convert a YUV YV12 planar image to RGB uchar3.
 */
cl_int oclYUYVToRGB8(ocl_program& prg, cl_mem input, cl_mem output, size_t width, size_t height);

/**
 * Convert a YUV YV12 planar image to RGB float3.
 */
cl_int oclYUYVToRGB32F(ocl_program& prg, cl_mem input, cl_mem output, size_t width, size_t height);

/**
 * Convert a YUV YV12 planar image to RGBA uchar4.
 */
cl_int oclYUYVToRGBA8(ocl_program& prg, cl_mem input, cl_mem output, size_t width, size_t height);

/**
 * Convert a YUV YV12 planar image to RGB float4.
 */
cl_int oclYUYVToRGBA32F(ocl_program& prg, cl_mem input, cl_mem output, size_t width, size_t height);

///@}


// cudaUYVYToRGB (uchar3)
cl_int oclUYVYToRGB8(ocl_program& prg, cl_mem input, cl_mem output, size_t width, size_t height );

// oclUYVYToRGB (float3)
cl_int oclUYVYToRGB32F(ocl_program& prg,  cl_mem input, cl_mem output, size_t width, size_t height );

// oclUYVYToRGBA (uchar4)
cl_int oclUYVYToRGBA8(ocl_program& prg,  cl_mem input, cl_mem output, size_t width, size_t height );

// oclUYVYToRGBA (float4)
cl_int oclUYVYToRGBA32F(ocl_program& prg,  cl_mem input, cl_mem output, size_t width, size_t height );


cl_int oclYVYUToRGB8(ocl_program& prg, cl_mem input, cl_mem output, size_t width, size_t height );
cl_int oclYVYUToRGB32F(ocl_program& prg, cl_mem input, cl_mem output, size_t width, size_t height );
cl_int oclYVYUToRGBA8(ocl_program& prg, cl_mem input, cl_mem output, size_t width, size_t height );
cl_int oclYVYUToRGBA32F(ocl_program& prg, cl_mem input, cl_mem output, size_t width, size_t height );
