/*
    Copyright 2011 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */

#ifndef SkAutoCo_DEFINED
#define SkAutoCo_DEFINED

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "SkTemplates.h"

/**
 * An instance of this class initializes COM on creation
 * and closes the COM library on destruction.
 */
class AutoCoInitialize : SkNoncopyable {
private:
    HRESULT fHR;
public:
    AutoCoInitialize();
    ~AutoCoInitialize();
    HRESULT getHR();
};

#endif
