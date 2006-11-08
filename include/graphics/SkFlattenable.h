/* include/graphics/SkFlattenable.h
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

#ifndef SkFlattenable_DEFINED
#define SkFlattenable_DEFINED

#include "SkRefCnt.h"
#include "SkBuffer.h"

/** \class SkFlattenable

    SkFlattenable is the base class for objects that need to be flattened
    into a data stream for either transport or as part of the key to the
    font cache.
*/
//  This class is not exported to java.
class SkFlattenable : public SkRefCnt {
public:
    typedef SkFlattenable* (*Factory)(SkRBuffer&);

    virtual Factory getFactory();
    virtual void    flatten(SkWBuffer&);
};

#endif

