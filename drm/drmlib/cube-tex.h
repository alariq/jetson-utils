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

#ifndef _CUBE_TEX_H
#define _CUBE_TEX_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "common.h"
#include "esUtil.h"

WEAK uint64_t
gbm_bo_get_modifier(struct gbm_bo *bo);

int get_fd_rgba(uint32_t *pstride, uint64_t *modifier);
int get_fd_y(uint32_t *pstride, uint64_t *modifier);
int get_fd_uv(uint32_t *pstride, uint64_t *modifier);

#ifdef __cplusplus
extern "C" {
#endif
int init_tex_rgba_ex(struct gbm_device *gbm_dev, const struct egl *egl, uint32_t tw, uint32_t th, bool b_fill, GLuint* tex);
#ifdef __cplusplus
}
#endif

int init_tex_nv12_2img(void);
int init_tex_nv12_1img(void);
int init_tex(enum mode mode);
void draw_cube_tex(unsigned i);
const struct egl * init_cube_tex(const struct gbm *gbm, enum mode mode, int samples);

#endif // _CUBE_TEX_H
