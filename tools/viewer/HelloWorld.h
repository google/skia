/*
* Copyright 2017 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef HelloWorld_DEFINED
#define HelloWorld_DEFINED

#include "sk_app/Application.h"
#include "sk_app/CommandSet.h"
#include "sk_app/Window.h"

class SkCanvas;

class HelloWorld : public sk_app::Application {
public:
    HelloWorld(int argc, char** argv, void* platformData);
    ~HelloWorld() override;

    void onBackendCreated();
    void onPaint(SkCanvas* canvas);
    void onIdle() override;
    bool onKey(sk_app::Window::Key key, sk_app::Window::InputState state, uint32_t modifiers);
    bool onChar(SkUnichar c, uint32_t modifiers);

private:
    void updateTitle();

    sk_app::Window* fWindow;
    sk_app::CommandSet fCommands;
};

#endif
