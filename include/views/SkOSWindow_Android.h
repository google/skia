/*
 * Copyright (C) 2011 Skia
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SkOSWindow_Android_DEFINED
#define SkOSWindow_Android_DEFINED

#include "SkWindow.h"

class SkIRect;

class SkOSWindow : public SkWindow {
public:
    SkOSWindow(void*) {}
    ~SkOSWindow() {}
    bool attachGL() { return true; }
    void detachGL() {}
    void presentGL() {}

    virtual void onPDFSaved(const char title[], const char desc[],
        const char path[]);

protected:
    // overrides from SkWindow
    virtual void onHandleInval(const SkIRect&);
    virtual void onSetTitle(const char title[]);

private:
    typedef SkWindow INHERITED;
};

#endif

