//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "SampleApplication.h"

#include <cmath>
#include <algorithm>
#include <vector>

#include "Matrix.h"
#include "random_utils.h"
#include "shader_utils.h"

class MultiWindowSample : public SampleApplication
{
  public:
    MultiWindowSample()
        : SampleApplication("MultiWindow", 256, 256)
    {
    }

    virtual bool initialize()
    {
        const std::string vs = SHADER_SOURCE
        (
            attribute vec4 vPosition;
            void main()
            {
                gl_Position = vPosition;
            }
        );

        const std::string fs = SHADER_SOURCE
        (
            precision mediump float;
            void main()
            {
                gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
            }
        );

        mProgram = CompileProgram(vs, fs);
        if (!mProgram)
        {
            return false;
        }

        // Set an initial rotation
        mRotation = 45.0f;

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        window rootWindow;
        rootWindow.osWindow = getWindow();
        rootWindow.surface = getSurface();
        mWindows.push_back(rootWindow);

        const size_t numWindows = 5;
        for (size_t i = 1; i < numWindows; i++)
        {
            window window;

            window.osWindow = CreateOSWindow();
            if (!window.osWindow->initialize("MultiWindow", 256, 256))
            {
                return false;
            }

            window.surface = eglCreateWindowSurface(getDisplay(), getConfig(), window.osWindow->getNativeWindow(), nullptr);
            if (window.surface == EGL_NO_SURFACE)
            {
                return false;
            }

            window.osWindow->setVisible(true);

            mWindows.push_back(window);
        }

        for (size_t i = 1; i < mWindows.size(); i++)
        {
            int x = rootWindow.osWindow->getX() + static_cast<int>(RandomBetween(0, 512));
            int y = rootWindow.osWindow->getY() + static_cast<int>(RandomBetween(0, 512));
            int width = static_cast<int>(RandomBetween(128, 512));
            int height = static_cast<int>(RandomBetween(128, 512));
            mWindows[i].osWindow->setPosition(x, y);
            mWindows[i].osWindow->resize(width, height);
        }

        return true;
    }

    virtual void destroy()
    {
        glDeleteProgram(mProgram);
    }

    virtual void step(float dt, double totalTime)
    {
        mRotation = fmod(mRotation + (dt * 40.0f), 360.0f);

        for (size_t i = 1; i < mWindows.size(); i++)
        {
            mWindows[i].osWindow->messageLoop();
        }
    }

    virtual void draw()
    {
        OSWindow* rootWindow = mWindows[0].osWindow;
        int left = rootWindow->getX();
        int right = rootWindow->getX() + rootWindow->getWidth();
        int top = rootWindow->getY();
        int bottom = rootWindow->getY() + rootWindow->getHeight();
        for (size_t i = 1; i < mWindows.size(); i++)
        {
            OSWindow* window = mWindows[i].osWindow;
            left = std::min(left, window->getX());
            right = std::max(right, window->getX() + window->getWidth());
            top = std::min(top, window->getY());
            bottom = std::max(bottom, window->getY() + window->getHeight());
        }

        float midX = (left + right) * 0.5f;
        float midY = (top + bottom) * 0.5f;

        Matrix4 modelMatrix = Matrix4::translate(Vector3(midX, midY, 0.0f)) *
                              Matrix4::rotate(mRotation, Vector3(0.0f, 0.0f, 1.0f)) *
                              Matrix4::translate(Vector3(-midX, -midY, 0.0f));
        Matrix4 viewMatrix = Matrix4::identity();

        for (size_t i = 0; i < mWindows.size(); i++)
        {
            OSWindow* window = mWindows[i].osWindow;
            EGLSurface surface = mWindows[i].surface;

            eglMakeCurrent(getDisplay(), surface, surface, getContext());

            Matrix4 orthoMatrix = Matrix4::ortho(static_cast<float>(window->getX()), static_cast<float>(window->getX() + window->getWidth()),
                                                 static_cast<float>(window->getY() + window->getHeight()), static_cast<float>(window->getY()),
                                                 0.0f, 1.0f);
            Matrix4 mvpMatrix = orthoMatrix * viewMatrix * modelMatrix;

            Vector3 vertices[] =
            {
                Matrix4::transform(mvpMatrix, Vector4(midX, static_cast<float>(top), 0.0f, 1.0f)),
                Matrix4::transform(mvpMatrix, Vector4(static_cast<float>(left), static_cast<float>(bottom), 0.0f, 1.0f)),
                Matrix4::transform(mvpMatrix, Vector4(static_cast<float>(right), static_cast<float>(bottom), 0.0f, 1.0f)),
            };

            // Set the viewport
            glViewport(0, 0, window->getWidth(), window->getHeight());

            // Clear the color buffer
            glClear(GL_COLOR_BUFFER_BIT);

            // Use the program object
            glUseProgram(mProgram);

            // Load the vertex data
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices[0].data());
            glEnableVertexAttribArray(0);

            glDrawArrays(GL_TRIANGLES, 0, 3);

            eglSwapBuffers(getDisplay(), surface);
        }
    }

  private:
    // Handle to a program object
    GLuint mProgram;

    // Current rotation
    float mRotation;

    // Window and surface data
    struct window
    {
        OSWindow* osWindow;
        EGLSurface surface;
    };
    std::vector<window> mWindows;
};

int main(int argc, char **argv)
{
    MultiWindowSample app;
    return app.run();
}
