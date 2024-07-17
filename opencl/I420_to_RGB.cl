#pragma OPENCL EXTENSION cl_khr_fp16 : enable
#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable

#define IMAGE_RGB8 0
#define	IMAGE_RGBA8 1
#define IMAGE_RGB32F 2
#define IMAGE_RGBA32F 3

// BGR
#define IMAGE_BGR8 4
#define IMAGE_BGRA8 5
#define IMAGE_BGR32F 6
#define IMAGE_BGRA32F 7
	
// YUV
#define IMAGE_YUYV	8
#define IMAGE_YUY2 IMAGE_YUYV
#define IMAGE_YVYU 9
#define IMAGE_UYVY 10
#define IMAGE_I420 11
#define IMAGE_YV12 12
#define IMAGE_NV12 13
	
// Bayer
#define IMAGE_BAYER_BGGR 14
#define IMAGE_BAYER_GBRG 15
#define IMAGE_BAYER_GRBG 16
#define IMAGE_BAYER_RGGB 17
	
// grayscale
#define IMAGE_GRAY8 18
#define IMAGE_GRAY32F 19

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
    return (float3)(clamp(Y + 1.402f * V),
            clamp(Y - 0.344f * U - 0.714f * V),
            clamp(Y + 1.772f * U));
#else
    return (float3)(clamp(Y + 1.140f * V),
            clamp(Y - 0.395f * U - 0.581f * V),
            clamp(Y + 2.3032f * U));
#endif
}

__kernel void I420ToRGB(
        __global const uchar *src,
        int srcPitch,
        __global uchar *dst,
        int dstPitch,
        int width, int height, int num_channels)
{
    
    int x = get_global_id(0);
    int y = get_global_id(1);

	if( x >= width || y >= height )
		return;

	const int x2 = x/2;
	const int y2 = y/2;

	const int srcPitch2 = srcPitch/2;
	const int planeSize = srcPitch * height;

	// get the YUV plane offsets
	__global const uchar* y_plane = src;
	__global const uchar* u_plane;
	__global const uchar* v_plane;

#if !IS_I420
    v_plane = y_plane + planeSize;		
    u_plane = v_plane + (planeSize / 4);	// size of U & V planes is 25% of Y plane
#else                                            
    u_plane = y_plane + planeSize;		// in I420, order of U & V planes is reversed
    v_plane = u_plane + (planeSize / 4);
#endif

	// read YUV pixel
	const float Y = y_plane[y * srcPitch + x];
	const float U = u_plane[y2 * srcPitch2 + x2];
	const float V = v_plane[y2 * srcPitch2 + x2];

	const float3 RGB = YUV2RGB(Y, U, V);

    // Somehow we need to swap RGB -> BGR, not sure why, for CUDA it is ok
	dst[num_channels*(y * width + x) + 0] = (uchar)RGB.x;
	dst[num_channels*(y * width + x) + 1] = (uchar)RGB.y;
	dst[num_channels*(y * width + x) + 2] = (uchar)RGB.z;
    if(num_channels==4) {
        dst[num_channels*(y * width + x) + 3] = 255;
    }
}

//-------------------------------------------------------------------------------------
// RGB to I420/YV12
//-------------------------------------------------------------------------------------
inline uchar rgb_to_y(const uchar r, const uchar g, const uchar b)
{
	return (uchar)(((int)(30 * r) + (int)(59 * g) + (int)(11 * b)) / 100);
}

inline uchar3 rgb_to_yuv(const uchar r, const uchar g, const uchar b)
{
    uchar3 yuv;
	yuv.s0 = rgb_to_y(r, g, b);
	yuv.s1 = (uchar)(((int)(-17 * r) - (int)(33 * g) + (int)(50 * b) + 12800) / 100);
	yuv.s2 = (uchar)(((int)(50 * r) - (int)(42 * g) - (int)(8 * b) + 12800) / 100);
    return yuv;
}

#if RGB_TO_YV12
__kernel void RGBToYV12(__global const uchar* src, int srcAlignedWidth, 
        __global uchar* dst, int dstPitch, int width, int height, int n_ch )
{
	const int x = get_global_id(0) * 2;
	const int y = get_global_id(1) * 2;

	const int x1 = x + 1;
	const int y1 = y + 1;

	if( x1 >= width || y1 >= height )
		return;

	const int planeSize = height * dstPitch;
	
	__global uchar* y_plane = dst;
	__global uchar* u_plane;
	__global uchar* v_plane;

#if IS_I420
		u_plane = y_plane + planeSize;
		v_plane = u_plane + (planeSize / 4);	// size of U & V planes is 25% of Y plane
#else
		v_plane = y_plane + planeSize;		// in I420, order of U & V planes is reversed
		u_plane = v_plane + (planeSize / 4);
#endif

    uchar px_r;
	uchar px_g;
	uchar px_b;

    uchar y_val;

	px_r = src[n_ch*(y * srcAlignedWidth + x) + 0];
	px_g = src[n_ch*(y * srcAlignedWidth + x) + 1];
	px_b = src[n_ch*(y * srcAlignedWidth + x) + 2];
    y_val = rgb_to_y(px_r, px_g, px_b);
	//y_plane[y * dstPitch + x] = 255;//y_val;
	y_plane[y * dstPitch + x] = y_val;

	px_r = src[n_ch*(y * srcAlignedWidth + x1) + 0];
	px_g = src[n_ch*(y * srcAlignedWidth + x1) + 1];
	px_b = src[n_ch*(y * srcAlignedWidth + x1) + 2];
	y_val = rgb_to_y(px_r, px_g, px_b);
	//y_plane[y * dstPitch + x1] = 255;//y_val;
	y_plane[y * dstPitch + x1] = y_val;

	px_r = src[n_ch*(y1 * srcAlignedWidth + x) + 0];
	px_g = src[n_ch*(y1 * srcAlignedWidth + x) + 1];
	px_b = src[n_ch*(y1 * srcAlignedWidth + x) + 2];
	y_val = rgb_to_y(px_r, px_g, px_b);
	//y_plane[y1 * dstPitch + x] = 255;//y_val;
	y_plane[y1 * dstPitch + x] = y_val;
	
	px_r = src[n_ch*(y1 * srcAlignedWidth + x1) + 0];
	px_g = src[n_ch*(y1 * srcAlignedWidth + x1) + 1];
	px_b = src[n_ch*(y1 * srcAlignedWidth + x1) + 2];
	uchar3 yuv = rgb_to_yuv(px_r, px_g, px_b);
	//y_plane[y1 * dstPitch + x1] = 255;//yuv.s0;
	y_plane[y1 * dstPitch + x1] = yuv.s0;

	const int uvPitch = dstPitch / 2;
	const int uvIndex = (y / 2) * uvPitch + (x / 2);

	//u_plane[uvIndex] = 255;//yuv.s1;
	//v_plane[uvIndex] = 255;//yuv.s2;
	u_plane[uvIndex] = yuv.s1;
	v_plane[uvIndex] = yuv.s2;
} 
#endif

