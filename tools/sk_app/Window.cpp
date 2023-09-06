/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "tools/sk_app/Window.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "tools/window/WindowContext.h"

#if defined(SK_GANESH)
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#endif

namespace sk_app {

Window::Window() {}

Window::~Window() {}

void Window::detach() { fWindowContext = nullptr; }

void Window::visitLayers(std::function<void(Layer*)> visitor) {
    for (int i = 0; i < fLayers.size(); ++i) {
        if (fLayers[i]->fActive) {
            visitor(fLayers[i]);
        }
    }
}

bool Window::signalLayers(std::function<bool(Layer*)> visitor) {
    for (int i = fLayers.size() - 1; i >= 0; --i) {
        if (fLayers[i]->fActive && visitor(fLayers[i])) {
            return true;
        }
    }
    return false;
}

void Window::onBackendCreated() {
    this->visitLayers([](Layer* layer) { layer->onBackendCreated(); });
}

bool Window::onChar(SkUnichar c, skui::ModifierKey modifiers) {
    return this->signalLayers([=](Layer* layer) { return layer->onChar(c, modifiers); });
}

bool Window::onKey(skui::Key key, skui::InputState state, skui::ModifierKey modifiers) {
    return this->signalLayers([=](Layer* layer) { return layer->onKey(key, state, modifiers); });
}

bool Window::onMouse(int x, int y, skui::InputState state, skui::ModifierKey modifiers) {
    return this->signalLayers([=](Layer* layer) { return layer->onMouse(x, y, state, modifiers); });
}

bool Window::onMouseWheel(float delta, int x, int y, skui::ModifierKey modifiers) {
    return this->signalLayers(
            [=](Layer* layer) { return layer->onMouseWheel(delta, x, y, modifiers); });
}

bool Window::onTouch(intptr_t owner, skui::InputState state, float x, float y) {
    return this->signalLayers([=](Layer* layer) { return layer->onTouch(owner, state, x, y); });
}

bool Window::onFling(skui::InputState state) {
    return this->signalLayers([=](Layer* layer) { return layer->onFling(state); });
}

bool Window::onPinch(skui::InputState state, float scale, float x, float y) {
    return this->signalLayers([=](Layer* layer) { return layer->onPinch(state, scale, x, y); });
}

void Window::onUIStateChanged(const SkString& stateName, const SkString& stateValue) {
    this->visitLayers([=](Layer* layer) { layer->onUIStateChanged(stateName, stateValue); });
}

void Window::onPaint() {
    if (!fWindowContext) {
        return;
    }
    if (!fIsActive) {
        return;
    }
    sk_sp<SkSurface> backbuffer = fWindowContext->getBackbufferSurface();
    if (backbuffer == nullptr) {
        printf("no backbuffer!?\n");
        // TODO: try recreating testcontext
        return;
    }

    markInvalProcessed();

    // draw into the canvas of this surface
    this->visitLayers([](Layer* layer) { layer->onPrePaint(); });
    this->visitLayers([=](Layer* layer) { layer->onPaint(backbuffer.get()); });

    if (auto dContext = this->directContext()) {
        dContext->flushAndSubmit(backbuffer.get(), GrSyncCpu::kNo);
    }

    fWindowContext->swapBuffers();
}

void Window::onResize(int w, int h) {
    if (!fWindowContext) {
        return;
    }
    fWindowContext->resize(w, h);
    this->visitLayers([=](Layer* layer) { layer->onResize(w, h); });
}

void Window::onActivate(bool isActive) {
    if (fWindowContext) {
        fWindowContext->activate(isActive);
    }
    fIsActive = isActive;
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

GrDirectContext* Window::directContext() const {
    if (!fWindowContext) {
        return nullptr;
    }
    return fWindowContext->directContext();
}

skgpu::graphite::Context* Window::graphiteContext() const {
#if defined(SK_GRAPHITE)
    if (!fWindowContext) {
        return nullptr;
    }
    return fWindowContext->graphiteContext();
#else
    return nullptr;
#endif
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
