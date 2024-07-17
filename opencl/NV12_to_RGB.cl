#pragma OPENCL EXTENSION cl_khr_fp16 : enable
#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable

#define COLOR_COMPONENT_MASK            0x3FF
//#define COLOR_COMPONENT_BIT_SIZE        10
#define COLOR_COMPONENT_BIT_SIZE        10

#define FIXED_DECIMAL_POINT             24
#define FIXED_POINT_MULTIPLIER          1.0f
#define FIXED_COLOR_COMPONENT_MASK      0xffffffff


//-----------------------------------------------------------------------------------
// YUV to RGB colorspace conversion
//-----------------------------------------------------------------------------------
float clamp( float x )	
{ 
	return min(max(x, 0.0f), 255.0f); 
}

float3 YUV2RGB(uint3 yuvi)
{
	const float luma = (float)(yuvi.x);
	const float u    = (float)(yuvi.y) - 512.0f;
	const float v    = (float)(yuvi.z) - 512.0f;
	const float s    = 1.0f / 1024.0f * 255.0f;	// TODO clamp for uchar output?

#if 1
	return (float3)(clamp((luma + 1.402f * v) * s),
				    clamp((luma - 0.344f * u - 0.714f * v) * s),
				    clamp((luma + 1.772f * u) * s));
#else
	return (float3)(clamp((luma + 1.140f * v) * s),
				    clamp((luma - 0.395f * u - 0.581f * v) * s),
				    clamp((luma + 2.032f * u) * s));
#endif
}

__kernel void NV12ToRGB(
        __global const uint* srcImage, int nSourcePitch,
        __global uchar *dstImage, int nDestPitch,
        int width,
        int height,
        int num_ch)
{
	uint yuv101010Pel[2];
	uint processingPitch = ((width) + 63) & ~63;
	__global uchar* srcImageU8     = (__global uchar*)srcImage;

	processingPitch = nSourcePitch;
    
    int x = get_group_id(0) * (get_local_size(0) << 1) + (get_local_id(0) << 1);
    int y = get_global_id(1);

	if( x >= width || y >= height )
		return;

	// Read 2 Luma components at a time, so we don't waste processing since CbCr are decimated this way.
	// if we move to texture we could read 4 luminance values
	yuv101010Pel[0] = (srcImageU8[y * processingPitch + x    ]) << 2;
	yuv101010Pel[1] = (srcImageU8[y * processingPitch + x + 1]) << 2;

	uint chromaOffset    = processingPitch * height;
	int y_chroma = y >> 1;

	if (y & 1)  // odd scanline ?
	{
		uint chromaCb;
		uint chromaCr;

		chromaCb = srcImageU8[chromaOffset + y_chroma * processingPitch + x    ];
		chromaCr = srcImageU8[chromaOffset + y_chroma * processingPitch + x + 1];

		if (y_chroma < ((height >> 1) - 1)) // interpolate chroma vertically
		{
			chromaCb = (chromaCb + srcImageU8[chromaOffset + (y_chroma + 1) * processingPitch + x    ] + 1) >> 1;
			chromaCr = (chromaCr + srcImageU8[chromaOffset + (y_chroma + 1) * processingPitch + x + 1] + 1) >> 1;
		}

		yuv101010Pel[0] |= (chromaCb << (COLOR_COMPONENT_BIT_SIZE       + 2));
		yuv101010Pel[0] |= (chromaCr << ((COLOR_COMPONENT_BIT_SIZE << 1) + 2));

		yuv101010Pel[1] |= (chromaCb << (COLOR_COMPONENT_BIT_SIZE       + 2));
		yuv101010Pel[1] |= (chromaCr << ((COLOR_COMPONENT_BIT_SIZE << 1) + 2));
	}
    else
    {
		yuv101010Pel[0] |= ((uint)srcImageU8[chromaOffset + y_chroma * processingPitch + x    ] << (COLOR_COMPONENT_BIT_SIZE       + 2));
		yuv101010Pel[0] |= ((uint)srcImageU8[chromaOffset + y_chroma * processingPitch + x + 1] << ((COLOR_COMPONENT_BIT_SIZE << 1) + 2));

		yuv101010Pel[1] |= ((uint)srcImageU8[chromaOffset + y_chroma * processingPitch + x    ] << (COLOR_COMPONENT_BIT_SIZE       + 2));
		yuv101010Pel[1] |= ((uint)srcImageU8[chromaOffset + y_chroma * processingPitch + x + 1] << ((COLOR_COMPONENT_BIT_SIZE << 1) + 2));
	}

	// this steps performs the color conversion
    const uint3 yuvi_0 = (uint3)((yuv101010Pel[0] &   COLOR_COMPONENT_MASK),
                                ((yuv101010Pel[0] >>  COLOR_COMPONENT_BIT_SIZE)       & COLOR_COMPONENT_MASK),
                                ((yuv101010Pel[0] >> (COLOR_COMPONENT_BIT_SIZE << 1)) & COLOR_COMPONENT_MASK));

    const uint3 yuvi_1 = (uint3)((yuv101010Pel[1] &   COLOR_COMPONENT_MASK),
                                ((yuv101010Pel[1] >>  COLOR_COMPONENT_BIT_SIZE)       & COLOR_COMPONENT_MASK),
                                ((yuv101010Pel[1] >> (COLOR_COMPONENT_BIT_SIZE << 1)) & COLOR_COMPONENT_MASK));

    const float3 p0 = YUV2RGB(yuvi_0);
    const float3 p1 = YUV2RGB(yuvi_1);
								   
    int i=0;

    dstImage[num_ch * (y * width + x) + i++] = (uchar)p0.z;
    dstImage[num_ch * (y * width + x) + i++] = (uchar)p0.y;
    dstImage[num_ch * (y * width + x) + i++] = (uchar)p0.x;
    if(num_ch==4) {
        dstImage[num_ch*(y * width + x) + i++] = 255;
    }

	dstImage[num_ch*(y * width + x) + i++] = (uchar)p1.z;
	dstImage[num_ch*(y * width + x) + i++] = (uchar)p1.y;
	dstImage[num_ch*(y * width + x) + i++] = (uchar)p1.x;
    if(num_ch==4) {
        dstImage[num_ch*(y * width + x) + i++] = 255;
    }

}

