// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file is auto-generated from build_workaround_header.py
// DO NOT EDIT!

#define GPU_DRIVER_BUG_WORKAROUNDS(GPU_OP)              \
  GPU_OP(DISABLE_BLEND_EQUATION_ADVANCED,               \
         disable_blend_equation_advanced)               \
  GPU_OP(DISABLE_DISCARD_FRAMEBUFFER,                   \
         disable_discard_framebuffer)                   \
  GPU_OP(DISALLOW_LARGE_INSTANCED_DRAW,                 \
         disallow_large_instanced_draw)                 \
  GPU_OP(GL_CLEAR_BROKEN,                               \
         gl_clear_broken)                               \
  GPU_OP(MAX_MSAA_SAMPLE_COUNT_4,                       \
         max_msaa_sample_count_4)                       \
  GPU_OP(MAX_TEXTURE_SIZE_LIMIT_4096,                   \
         max_texture_size_limit_4096)                   \
  GPU_OP(PACK_PARAMETERS_WORKAROUND_WITH_PACK_BUFFER,   \
         pack_parameters_workaround_with_pack_buffer)   \
  GPU_OP(RESTORE_SCISSOR_ON_FBO_CHANGE,                 \
         restore_scissor_on_fbo_change)                 \
  GPU_OP(UNBIND_ATTACHMENTS_ON_BOUND_RENDER_FBO_DELETE, \
         unbind_attachments_on_bound_render_fbo_delete) \
// The End
