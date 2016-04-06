/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef VulkanViewer_DEFINED
#define VulkanViewer_DEFINED

#include "../Application.h"
#include "InputHandler.h"
#include "../Window.h"
#include "gm.h"

class SkCanvas;

class VulkanViewer : public Application {
public:
    VulkanViewer(int argc, char** argv, void* platformData);
    ~VulkanViewer() override;

    bool onKey(int key, bool keyDown);
    bool onMouse(int x, int y, bool mouseDown);
    void onPaint(SkCanvas* canvas);

    void onIdle(float dt) override;

private:
    Window*      fWindow;
    InputHandler fInputHandler;

    const skiagm::GMRegistry* fGMs;
};


#endif
