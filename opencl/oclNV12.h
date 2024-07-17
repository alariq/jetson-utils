#pragma once

#include "oclUtility.h"
#include "ocl_program.h"


//////////////////////////////////////////////////////////////////////////////////
/// @name YUV NV12 4:2:0 to RGB
/// @see cudaConvertColor() from cudaColorspace.h for automated format conversion
/// @ingroup colorspace
//////////////////////////////////////////////////////////////////////////////////

///@{

/**
 * Convert an NV12 texture (semi-planar 4:2:0) to RGB uchar3 format.
 * NV12 = 8-bit Y plane followed by an interleaved U/V plane with 2x2 subsampling.
 */
cl_int oclNV12ToRGB8(ocl_program& p, cl_mem input, cl_mem output, size_t width, size_t height );

/**
 * Convert an NV12 texture (semi-planar 4:2:0) to RGB float3 format.
 * NV12 = 8-bit Y plane followed by an interleaved U/V plane with 2x2 subsampling.
 */
cl_int oclNV12ToRGB32F(ocl_program& p, cl_mem input, cl_mem output, size_t width, size_t height );

/**
 * Convert an NV12 texture (semi-planar 4:2:0) to RGBA uchar4 format.
 * NV12 = 8-bit Y plane followed by an interleaved U/V plane with 2x2 subsampling.
 */
cl_int oclNV12ToRGBA8(ocl_program& p, cl_mem input, cl_mem output, size_t width, size_t height );

/**
 * Convert an NV12 texture (semi-planar 4:2:0) to RGBA float4 format.
 * NV12 = 8-bit Y plane followed by an interleaved U/V plane with 2x2 subsampling.
 */
cl_int oclNV12ToRGBA32F(ocl_program& p, cl_mem input, cl_mem output, size_t width, size_t height );

///@}


