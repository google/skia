/*-------------------------------------------------------------------------
 * drawElements Quality Program Tester Core
 * ----------------------------------------
 *
 * Copyright 2014 The Android Open Source Project
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
 *
 */

#ifndef TCU_ANGLE_WIN32_NATIVE_DISPLAY_FACTORY_H_
#define TCU_ANGLE_WIN32_NATIVE_DISPLAY_FACTORY_H_

#include "tcuDefs.hpp"
#include "egluNativeDisplay.hpp"
#include "eglwDefs.hpp"

namespace tcu
{

class EventState
{
  public:
    EventState()
        : mQuit(false)
    {
    }
    bool quitSignaled() const { return mQuit; };
    void signalQuitEvent() { mQuit = true; };

  private:
    bool mQuit;
};

class ANGLENativeDisplayFactory : public eglu::NativeDisplayFactory
{
  public:
    ANGLENativeDisplayFactory(const std::string &name,
                              const std::string &description,
                              const std::vector<eglw::EGLAttrib> &platformAttributes,
                              EventState *eventState);
    ~ANGLENativeDisplayFactory() override;

    eglu::NativeDisplay *createDisplay(const eglw::EGLAttrib* attribList) const override;

  private:
    std::vector<eglw::EGLAttrib> mPlatformAttributes;
};

} // tcu

#endif // TCU_ANGLE_WIN32_NATIVE_DISPLAY_FACTORY_H_
