#pragma OPENCL EXTENSION cl_khr_fp16 : enable
#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable

//-----------------------------------------------------------------------------------
// YUV to RGB colorspace conversion
//-----------------------------------------------------------------------------------
float clamp( float x )	
{ 
	return min(max(x, 0.0f), 255.0f); 
}

float3 YUV2RGB(float Y, float U, float V)
{
	U -= 128.0f;
	V -= 128.0f;

#if 1
    return (float3)(
            clamp(Y + 1.4065f * V),
            clamp(Y - 0.3455f * U - 0.7169f * V),
            clamp(Y + 1.7790f * U)
            );
#else
	return make_float3(clamp(Y + 1.402f * V),
    				    clamp(Y - 0.344f * U - 0.714f * V),
				    clamp(Y + 1.772f * U));
#endif
}

__kernel void YUYVToRGB(
        __global const uchar4 *src,
        __global uchar *dst,
        int halfWidth,
        int height,
        int num_ch)
{
    
    int x = get_global_id(0);
    int y = get_global_id(1);

	if( x >= halfWidth || y >= height )
		return;

	const uchar4 macroPx = src[y * halfWidth + x];

	// Y0 is the brightness of pixel 0, Y1 the brightness of pixel 1.
	// U and V is the color of both pixels.
	float y0, y1, u, v;

#if defined(FMT_YUYV)
	{
		// YUYV [ Y0 | U0 | Y1 | V0 ]
		y0 = macroPx.x;
		y1 = macroPx.z;
		u  = macroPx.y;
		v  = macroPx.w;
	}
#elif defined(FMT_YVYU)
	{
		// YVYU [ Y0 | V0 | Y1 | U0 ]
		y0 = macroPx.x;
		y1 = macroPx.z;
		u  = macroPx.w;
		v  = macroPx.y;
	}
#else
	// if( format == IMAGE_UYVY )
	{
		// UYVY [ U0 | Y0 | V0 | Y1 ]
		y0 = macroPx.y;
		y1 = macroPx.w;
		u  = macroPx.x;
		v  = macroPx.z;
	}
#endif

	// this function outputs two pixels from one YUYV macropixel
	const float3 px0 = YUV2RGB(y0, u, v);
	const float3 px1 = YUV2RGB(y1, u, v);

    int i=0;
	dst[2*num_ch*(y * halfWidth + x) + i++] = (uchar)px0.x;
	dst[2*num_ch*(y * halfWidth + x) + i++] = (uchar)px0.y;
	dst[2*num_ch*(y * halfWidth + x) + i++] = (uchar)px0.z;
    if(num_ch==4) {
        dst[2*num_ch*(y * halfWidth + x) + i++] = 255;
    }

	dst[2*num_ch*(y * halfWidth + x) + i++] = (uchar)px1.x;
	dst[2*num_ch*(y * halfWidth + x) + i++] = (uchar)px1.y;
	dst[2*num_ch*(y * halfWidth + x) + i++] = (uchar)px1.z;
    if(num_ch==4) {
        dst[2*num_ch*(y * halfWidth + x) + i++] = 255;
    }
}

