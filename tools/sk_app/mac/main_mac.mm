/*
* Copyright 2019 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GLFW/glfw3.h"
#include "tools/sk_app/Application.h"
#include "tools/sk_app/mac/Window_mac.h"

///////////////////////////////////////////////////////////////////////////////////////////

using sk_app::Application;
using sk_app::Window_mac;

extern int gWindowCount;

int main(int argc, char * argv[]) {
    if (!glfwInit())
    {
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GL_TRUE);

    Application* app = Application::Create(argc, argv, nullptr);

    // need a stopping condition here
    while (gWindowCount) {
        glfwPollEvents();

        app->onIdle();
    }

    delete app;

    glfwTerminate();

    return 0;
}
