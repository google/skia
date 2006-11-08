/* libs/graphics/text/ATextEntry.h
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

#ifndef ATextEntry_DEFINED
#define ATextEntry_DEFINED

#include "SkTypes.h"

class SkCanavs;

class ATextEntry {
public:
    ATextEntry();
    ~ATextEntry();
    
    void    setUtf16(const U16 text[], size_t count);
    void    setSize(SkScalar width, SkScalar height);
    void    setSelection(int start, int stop);
    void    draw(SkCanvas*);
    void    handleKey(int key);
};

#endif
