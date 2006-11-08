/* include/graphics/SkFontCodec.h
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

#ifndef SkFontCodec_DEFINED
#define SkFontCodec_DEFINED

#include "SkSFNT.h"

class SkFontCodec {
public:
    static void Compress(SkSFNT& font, const char fileName[]);

    /*  Format is [count] + [instruction, bitcount] * count
        Allocated with sk_malloc()
    */
    static U8* BuildInstrHuffmanTable(SkSFNT&);
    static U8* BuildOutlineHuffmanTable(SkSFNT& font);

    SkDEBUGCODE(static void UnitTest();)
};

#endif

