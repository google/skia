/* libs/graphics/animator/SkBoundable.h
**
** Copyright 2006, Google Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#ifndef SkBoundable_DEFINED
#define SkBoundable_DEFINED

#include "SkDrawable.h"
#include "SkRect.h"

class SkBoundable : public SkDrawable {
public:
    SkBoundable();
    virtual void clearBounder();
    virtual void enableBounder();
    virtual void getBounds(SkRect* );
    bool hasBounds() { return fBounds.fLeft != (S16)0x8000U; }
    void setBounds(SkRect16& bounds) { fBounds = bounds; }
protected:
    void clearBounds() { fBounds.fLeft = (S16) SkToU16(0x8000); }; // mark bounds as unset
    SkRect16 fBounds;
private:
    typedef SkDrawable INHERITED;
};

class SkBoundableAuto {
public:
    SkBoundableAuto(SkBoundable* boundable, SkAnimateMaker& maker);
    ~SkBoundableAuto();
private:
    SkBoundable* fBoundable;
    SkAnimateMaker& fMaker;
    SkBoundableAuto& operator= (const SkBoundableAuto& );
};

#endif // SkBoundable_DEFINED

