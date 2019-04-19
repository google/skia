/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "Window.h"

#include "SkSurface.h"
#include "SkCanvas.h"
#include "WindowContext.h"

namespace sk_app {

Window::Window() {}

void Window::detach() {
    delete fWindowContext;
    fWindowContext = nullptr;
}

void Window::visitLayers(std::function<void(Layer*)> visitor) {
    for (int i = 0; i < fLayers.count(); ++i) {
        if (fLayers[i]->fActive) {
            visitor(fLayers[i]);
        }
    }
}

bool Window::signalLayers(std::function<bool(Layer*)> visitor) {
    for (int i = fLayers.count() - 1; i >= 0; --i) {
        if (fLayers[i]->fActive && visitor(fLayers[i])) {
            return true;
        }
    }
    return false;
}

void Window::onBackendCreated() {
    this->visitLayers([](Layer* layer) { layer->onBackendCreated(); });
}

bool Window::onChar(SkUnichar c, uint32_t modifiers) {
    return this->signalLayers([=](Layer* layer) { return layer->onChar(c, modifiers); });
}

bool Window::onKey(Key key, InputState state, uint32_t modifiers) {
    return this->signalLayers([=](Layer* layer) { return layer->onKey(key, state, modifiers); });
}

bool Window::onMouse(int x, int y, InputState state, uint32_t modifiers) {
    return this->signalLayers([=](Layer* layer) { return layer->onMouse(x, y, state, modifiers); });
}

bool Window::onMouseWheel(float delta, uint32_t modifiers) {
    return this->signalLayers([=](Layer* layer) { return layer->onMouseWheel(delta, modifiers); });
}

bool Window::onTouch(intptr_t owner, InputState state, float x, float y) {
    return this->signalLayers([=](Layer* layer) { return layer->onTouch(owner, state, x, y); });
}

void Window::onUIStateChanged(const SkString& stateName, const SkString& stateValue) {
    this->visitLayers([=](Layer* layer) { layer->onUIStateChanged(stateName, stateValue); });
}

void Window::onPaint() {
    if (!fWindowContext) {
        return;
    }
    markInvalProcessed();
    this->visitLayers([](Layer* layer) { layer->onPrePaint(); });
    sk_sp<SkSurface> backbuffer = fWindowContext->getBackbufferSurface();
    if (backbuffer) {
        // draw into the canvas of this surface
        this->visitLayers([=](Layer* layer) { layer->onPaint(backbuffer.get()); });

        backbuffer->flush();

        fWindowContext->swapBuffers();
    } else {
        printf("no backbuffer!?\n");
        // try recreating testcontext
    }
}

void Window::onResize(int w, int h) {
    if (!fWindowContext) {
        return;
    }
    fWindowContext->resize(w, h);
    this->visitLayers([=](Layer* layer) { layer->onResize(w, h); });
}

int Window::width() const {
    if (!fWindowContext) {
        return 0;
    }
    return fWindowContext->width();
}

int Window::height() const {
    if (!fWindowContext) {
        return 0;
    }
    return fWindowContext->height();
}

void Window::setRequestedDisplayParams(const DisplayParams& params, bool /* allowReattach */) {
    fRequestedDisplayParams = params;
    if (fWindowContext) {
        fWindowContext->setDisplayParams(fRequestedDisplayParams);
    }
}

int Window::sampleCount() const {
    if (!fWindowContext) {
        return 0;
    }
    return fWindowContext->sampleCount();
}

int Window::stencilBits() const {
    if (!fWindowContext) {
        return -1;
    }
    return fWindowContext->stencilBits();
}

GrContext* Window::getGrContext() const {
    if (!fWindowContext) {
        return nullptr;
    }
    return fWindowContext->getGrContext();
}

void Window::inval() {
    if (!fWindowContext) {
        return;
    }
    if (!fIsContentInvalidated) {
        fIsContentInvalidated = true;
        onInval();
    }
}

void Window::markInvalProcessed() {
    fIsContentInvalidated = false;
}

}   // namespace sk_app
