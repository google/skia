/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSurface_Compute_DEFINED
#define SkSurface_Compute_DEFINED

//
// for now make sure it's defined
//

#if !defined(SK_SUPPORT_GPU_COMPUTE)
#define SK_SUPPORT_GPU_COMPUTE 1
#endif

//
//
//

#if SK_SUPPORT_GPU_COMPUTE

//
//
//

#include "SkSurface_Base.h"
#include "GrContext.h"
#include "SkRefCnt.h"

// Yuqian: It doesn't seem right to me for SkSurface_Compute to depend on SkWindow.
//         Maybe we should move MakeComputeBackedSurface(from SkWindow) to SkWindow.cpp.
// #include "SkWindow.h"

//
//
//

#include "SkContext_Compute.h"

//
//
//

//
//
//

class SkSurface_Compute : public SkSurface_Base
{
 public:

  SkSurface_Compute(sk_sp<SkContext_Compute> compute,
                    int const width, int const height);

  ~SkSurface_Compute();

  //
  //
  //

  SkCanvas*         onNewCanvas()                                    override;
  sk_sp<SkSurface>  onNewSurface(const SkImageInfo&)                 override;
  sk_sp<SkImage>    onNewImageSnapshot() override;
  void              onCopyOnWrite(ContentChangeMode)                 override;

  //
  //
  //

  // static sk_sp<SkSurface> MakeComputeBackedSurface(SkWindow                       * const window,
  //                                                  const SkWindow::AttachmentInfo &       attachmentInfo,
  //                                                  GrGLInterface const            * const grInterface,
  //                                                  GrContext                      * const grContext,
  //                                                  sk_sp<SkContext_Compute>               compute);
  // //
  //
  //

 private:

  typedef SkSurface_Base INHERITED;

  //
  //
  //

  sk_sp<SkBaseDevice>      device;

  //
  //
  //

  sk_sp<SkContext_Compute> compute;

  //
  //
  //
};

//
//
//

#endif

//
//
//

#endif
