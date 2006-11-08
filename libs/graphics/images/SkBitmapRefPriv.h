/* libs/graphics/images/SkBitmapRefPriv.h
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

#ifndef SkBitmapRefPriv_DEFINED
#define SkBitmapRefPriv_DEFINED

#include "SkBitmapRef.h"
#include "SkGlobals.h"
#include "SkThread.h"
#include "SkBitmap.h"
#include "SkString.h"

#define kBitmapRef_GlobalsTag   SkSetFourByteTag('s', 'k', 'b', 'r')

class SkBitmapRef_Globals : public SkGlobals::Rec {
public:
    SkMutex             fMutex;
    SkBitmapRef::Rec*   fCache;
    
    static Rec* Create();
};

struct SkBitmapRef::Rec {
    Rec(bool isCache): fIsCache(isCache) {}
    Rec(const SkBitmap& src): fBM(src) {}
    
    Rec*        fNext;
    int32_t     fRefCnt;
    SkString    fPath;
    SkBitmap    fBM;
    bool        fIsCache;
};

#endif
