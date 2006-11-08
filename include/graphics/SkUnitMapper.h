/* include/graphics/SkUnitMapper.h
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

#ifndef SkUnitMapper_DEFINED
#define SkUnitMapper_DEFINED

#include "SkRefCnt.h"
#include "SkScalar.h"

class SkUnitMapper : public SkRefCnt {
public:
    /** Given a value in [0..0xFFFF], return a value in the same range.
    */
    virtual U16CPU mapUnit16(U16CPU x) = 0;
};

/** This returns N values between [0...1]
*/
class SkDiscreteMapper : public SkUnitMapper {
public:
    SkDiscreteMapper(unsigned segments);
    // override
    virtual U16CPU mapUnit16(U16CPU x);
private:
    unsigned fSegments;
    SkFract fScale;
};

/** This returns 1 - cos(x), to simulate lighting a sphere
*/
class SkFlipCosineMapper : public SkUnitMapper {
public:
    // override
    virtual U16CPU mapUnit16(U16CPU x);
};

#endif

