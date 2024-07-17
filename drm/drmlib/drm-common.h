/*
 * Copyright (c) 2017 Rob Clark <rclark@redhat.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef _DRM_COMMON_H
#define _DRM_COMMON_H

#include <xf86drm.h>
#include <xf86drmMode.h>

struct gbm;
struct egl;

struct plane {
	drmModePlane *plane;
	drmModeObjectProperties *props;
	drmModePropertyRes **props_info;
};

struct crtc {
	drmModeCrtc *crtc;
	drmModeObjectProperties *props;
	drmModePropertyRes **props_info;
};

struct connector {
	drmModeConnector *connector;
	drmModeObjectProperties *props;
	drmModePropertyRes **props_info;
};

struct drm {
	int fd;

	/* only used for atomic: */
	struct plane *plane;
	struct crtc *crtc;
	struct connector *connector;
	int crtc_index;
	int kms_in_fence_fd;
	int kms_out_fence_fd;

	drmModeModeInfo *mode;
	uint32_t crtc_id;
	uint32_t connector_id;

	/* number of frames to run for: */
	unsigned int count;

	int (*run)(const struct gbm *gbm, const struct egl *egl);
};

struct drm_fb {
	struct gbm_bo *bo;
	uint32_t fb_id;
};

//sebi:
struct drm_dumb_fb {
	uint32_t id;     // DRM object ID
	uint32_t width;
	uint32_t height;
	uint32_t stride;
	uint32_t handle; // driver-specific handle
	uint64_t size;   // size of mapping

	uint8_t *data;   // mmapped data we can write to
};

struct rfc {
	struct gbm_bo *bo;
	struct drm_fb *fb;

	uint32_t i;
	int64_t start_time, report_time;
};

//


#ifdef __cplusplus
extern "C" {
#endif
struct drm_fb * drm_fb_get_from_bo(struct gbm_bo *bo);

int init_drm(struct drm *drm, const char *device, const char *mode_str, int connector_id, unsigned int vrefresh, unsigned int count);
int init_drm_render(struct drm *drm, const char *device, const char *mode_str, unsigned int count);
const struct drm * init_drm_legacy(const char *device, const char *mode_str, int connector_id, unsigned int vrefresh, unsigned int count);
const struct drm * init_drm_atomic(const char *device, const char *mode_str, int connector_id, unsigned int vrefresh, unsigned int count);
const struct drm * init_drm_offscreen(const char *device, const char *mode_str, unsigned int count);

//sebi:
bool create_drm_dumb_fb(int drm_fd, uint32_t width, uint32_t height, struct drm_dumb_fb* fb);
int free_drm_dumb_fb(uint32_t drm_fd, struct drm_dumb_fb* fb);
int legacy_run_setup(const struct gbm *gbm, const struct egl *egl, struct rfc* rfc);

#ifdef __cplusplus
}
#endif

#endif /* _DRM_COMMON_H */
