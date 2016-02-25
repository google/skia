/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#if SK_SUPPORT_GPU
#include "gl/GrGLGpu.h"
#include "GrContext.h"
#include "SkSurface_Gpu.h"
#include "../include/gpu/gl/SkGLContext.h"
#include "../include/gpu/GrTypes.h"
#include "../include/private/SkTemplates.h"

class TestStorageAllocator {
 public:
  static GrTextureStorageAllocator::Result allocateTextureStorage(void* ctx,
          GrBackendObject texture, unsigned width, unsigned height, GrPixelConfig config,
          const void* srcData, GrSurfaceOrigin) {
      TestStorageAllocator* allocator = static_cast<TestStorageAllocator*>(ctx);
      if (!allocator->m_allowAllocation)
          return GrTextureStorageAllocator::Result::kFailed;
      SkAutoTMalloc<uint8_t> pixels(width * height * 4);
      memset(pixels.get(), 0, width * height * 4);

      GrGLuint id;
      GrGLenum target = GR_GL_TEXTURE_2D;
      GR_GL_CALL(allocator->m_gl, GenTextures(1, &id));
      GR_GL_CALL(allocator->m_gl, BindTexture(target, id));
      GR_GL_CALL(allocator->m_gl, TexParameteri(target, GR_GL_TEXTURE_MAG_FILTER, GR_GL_NEAREST));
      GR_GL_CALL(allocator->m_gl, TexParameteri(target, GR_GL_TEXTURE_MIN_FILTER, GR_GL_NEAREST));
      GR_GL_CALL(allocator->m_gl, TexParameteri(target, GR_GL_TEXTURE_WRAP_S, GR_GL_CLAMP_TO_EDGE));
      GR_GL_CALL(allocator->m_gl, TexParameteri(target, GR_GL_TEXTURE_WRAP_T, GR_GL_CLAMP_TO_EDGE));
      GR_GL_CALL(allocator->m_gl, TexImage2D(target, 0, GR_GL_RGBA, width, height, 0, GR_GL_RGBA,
                                  GR_GL_UNSIGNED_BYTE, pixels.get()));

      GrGLTextureInfo* info = reinterpret_cast<GrGLTextureInfo*>(texture);
      info->fID = id;
      info->fTarget = target;
      allocator->m_mostRecentlyAllocatedStorage = id;
      return GrTextureStorageAllocator::Result::kSucceededWithoutUpload;
  }
  static void deallocateTextureStorage(void* ctx, GrBackendObject texture) {
      TestStorageAllocator* allocator = static_cast<TestStorageAllocator*>(ctx);
      GrGLTextureInfo* info = reinterpret_cast<GrGLTextureInfo*>(texture);
      GR_GL_CALL(allocator->m_gl, DeleteTextures(1, &(info->fID)));
  }

  GrGLuint m_mostRecentlyAllocatedStorage;
  const GrGLInterface* m_gl;
  bool m_allowAllocation;
};

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(CustomTexture, reporter, context, glContext) {
    static const int kWidth = 13;
    static const int kHeight = 13;

    const GrGLInterface* gl = glContext->gl();
    TestStorageAllocator allocator;
    allocator.m_allowAllocation = true;
    allocator.m_gl = gl;
    GrTextureStorageAllocator grAllocator;
    grAllocator.fAllocateTextureStorage = &TestStorageAllocator::allocateTextureStorage;
    grAllocator.fDeallocateTextureStorage= &TestStorageAllocator::deallocateTextureStorage;
    grAllocator.fCtx = &allocator;

    SkAutoTUnref<SkSurface> surface(SkSurface_Gpu::NewRenderTarget(
            context, SkBudgeted::kNo, SkImageInfo::MakeN32Premul(kWidth, kHeight), 0,
            NULL, grAllocator));
    REPORTER_ASSERT(reporter, surface);
    GrGLuint id = allocator.m_mostRecentlyAllocatedStorage;

    SkAutoTUnref<SkImage> image(surface->newImageSnapshot());
    REPORTER_ASSERT(reporter, image->isTextureBacked());
    SkImageInfo imageInfo = SkImageInfo::MakeN32Premul(1,1);
    GrColor dest = 0x11223344;
    REPORTER_ASSERT(reporter, image->readPixels(imageInfo, &dest, 4 * kWidth, 0, 0));
    REPORTER_ASSERT(reporter, GrColorUnpackG(dest) == 0);

    surface->getCanvas()->clear(SK_ColorGREEN);
    SkAutoTUnref<SkImage> image2(surface->newImageSnapshot());
    REPORTER_ASSERT(reporter, image2->isTextureBacked());
    REPORTER_ASSERT(reporter, allocator.m_mostRecentlyAllocatedStorage != id);

    REPORTER_ASSERT(reporter, image2->readPixels(imageInfo, &dest, 4 * kWidth, 0, 0));
    REPORTER_ASSERT(reporter, GrColorUnpackG(dest) == 255);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(CustomTextureFailure, reporter, context, glContext) {
    static const int kWidth = 13;
    static const int kHeight = 13;

    const GrGLInterface* gl = glContext->gl();
    TestStorageAllocator allocator;
    allocator.m_allowAllocation = false;
    allocator.m_gl = gl;
    GrTextureStorageAllocator grAllocator;
    grAllocator.fAllocateTextureStorage = &TestStorageAllocator::allocateTextureStorage;
    grAllocator.fDeallocateTextureStorage= &TestStorageAllocator::deallocateTextureStorage;
    grAllocator.fCtx = &allocator;
    SkAutoTUnref<SkSurface> surface(SkSurface_Gpu::NewRenderTarget(
            context, SkBudgeted::kNo, SkImageInfo::MakeN32Premul(kWidth, kHeight), 0,
            NULL, grAllocator));
    REPORTER_ASSERT(reporter, !surface);
}

#endif
