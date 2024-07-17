#pragma once

#include "oclUtility.h"
#include "ocl_program.h"

//////////////////////////////////////////////////////////////////////////////////
/// @name YUV I420 4:2:0 planar to RGB
/// @see oclConvertColor() from oclColorspace.h for automated format conversion
/// @ingroup colorspace
//////////////////////////////////////////////////////////////////////////////////

///@{

/**
 * Convert a YUV I420 planar image to RGB uchar3.
 */
cl_int oclI420ToRGB8(ocl_program& prg, cl_mem input, cl_mem output, size_t width, size_t height);

/**
 * Convert a YUV I420 planar image to RGB float3.
 */
cl_int oclI420ToRGB32F(ocl_program& prg, cl_mem input, cl_mem output, size_t width, size_t height);

/**
 * Convert a YUV I420 planar image to RGBA uchar4.
 */
cl_int oclI420ToRGBA8(ocl_program& prg, cl_mem input, cl_mem output, size_t width, size_t height);

/**
 * Convert a YUV I420 planar image to RGB float4.
 */
cl_int oclI420ToRGBA32F(ocl_program& prg, cl_mem input, cl_mem output, size_t width, size_t height);

///@}


//////////////////////////////////////////////////////////////////////////////////
/// @name YUV YV12 4:2:0 planar to RGB
/// @see cudaConvertColor() from cudaColorspace.h for automated format conversion
/// @ingroup colorspace
//////////////////////////////////////////////////////////////////////////////////

///@{

/**
 * Convert a YUV YV12 planar image to RGB uchar3.
 */
cl_int oclYV12ToRGB8(ocl_program& prg, cl_mem input, cl_mem output, size_t width, size_t height);

/**
 * Convert a YUV YV12 planar image to RGB float3.
 */
cl_int oclYV12ToRGB32F(ocl_program& prg, cl_mem input, cl_mem output, size_t width, size_t height);

/**
 * Convert a YUV YV12 planar image to RGBA uchar4.
 */
cl_int oclYV12ToRGBA8(ocl_program& prg, cl_mem input, cl_mem output, size_t width, size_t height);

/**
 * Convert a YUV YV12 planar image to RGB float4.
 */
cl_int oclYV12ToRGBA32F(ocl_program& prg, cl_mem input, cl_mem output, size_t width, size_t height);

///@}


//////////////////////////////////////////////////////////////////////////////////
/// @name RGB to YUV I420 4:2:0 planar
/// @see cudaConvertColor() from cudaColorspace.h for automated format conversion
/// @ingroup colorspace
//////////////////////////////////////////////////////////////////////////////////

///@{

/**
 * Convert an RGB uchar3 buffer into YUV I420 planar.
 */
cl_int oclRGB8ToI420(ocl_program& p, cl_mem input, cl_mem output, int width, int height );

/**
 * Convert an RGB float3 buffer into YUV I420 planar.
 */
cl_int oclRGB32FToI420(ocl_program& p, cl_mem input, cl_mem output, int width, int height );

/**
 * Convert an RGBA uchar4 buffer into YUV I420 planar.
 */
cl_int oclRGBA8ToI420(ocl_program& p,  cl_mem input, cl_mem output, size_t width, size_t height );

/**
 * Convert an RGBA float4 buffer into YUV I420 planar.
 */
cl_int oclRGBA32FToI420(ocl_program& p, cl_mem input, cl_mem output, size_t width, size_t height );

///@}

