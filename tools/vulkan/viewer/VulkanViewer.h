/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef VulkanViewer_DEFINED
#define VulkanViewer_DEFINED

#include "../Application.h"
#include "../Window.h"
#include "gm.h"

class SkCanvas;

class VulkanViewer : public Application {
public:
    VulkanViewer(int argc, char** argv, void* platformData);
    ~VulkanViewer() override;

    bool onKey(Window::Key key, Window::InputState state, uint32_t modifiers);
    void onPaint(SkCanvas* canvas);

    void onIdle(float dt) override;

private:
    Window*      fWindow;

    const skiagm::GMRegistry* fGMs;
};


#endif
