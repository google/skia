//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DebugAnnotator9.h: D3D9 helpers for adding trace annotations.
//

#ifndef LIBANGLE_RENDERER_D3D_D3D9_DEBUGANNOTATOR9_H_
#define LIBANGLE_RENDERER_D3D_D3D9_DEBUGANNOTATOR9_H_

#include "common/debug.h"

namespace rx
{

class DebugAnnotator9 : public gl::DebugAnnotator
{
  public:
    DebugAnnotator9() {}
    void beginEvent(const wchar_t *eventName) override;
    void endEvent() override;
    void setMarker(const wchar_t *markerName) override;
    bool getStatus() override;
};

}

#endif // LIBANGLE_RENDERER_D3D_D3D9_DEBUGANNOTATOR9_H_
