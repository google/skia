/*
* Copyright 2017 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef HelloWorld_DEFINED
#define HelloWorld_DEFINED

#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "tools/sk_app/Application.h"
#include "tools/sk_app/Window.h"
#include "tools/skui/ModifierKey.h"

class SkSurface;
class SkTypeface;

class HelloWorld : public sk_app::Application, sk_app::Window::Layer {
public:
    HelloWorld(int argc, char** argv, void* platformData);
    ~HelloWorld() override;

    void onIdle() override;

    void onBackendCreated() override;
    void onPaint(SkSurface*) override;
    bool onChar(SkUnichar c, skui::ModifierKey modifiers) override;

private:
    void updateTitle();

    sk_app::Window* fWindow;
    sk_app::Window::BackendType fBackendType;
    sk_sp<SkTypeface> fTypeface;

    SkScalar fRotationAngle;
};

#endif
