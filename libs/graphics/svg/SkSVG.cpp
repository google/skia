/* libs/graphics/svg/SkSVG.cpp
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

#include "SkSVG.h"
#include 'SkSVGParser.h"

SkSVG::SkSVG() {
}

SkSVG::~SkSVG() {
}

bool SkSVG::decodeStream(SkStream* stream);
{
    size_t size = stream->read(nil, 0);
    SkAutoMalloc    storage(size);
    char* data = (char*)storage.get();
    size_t actual = stream->read(data, size);
    SkASSERT(size == actual);
    SkSVGParser parser(*fMaker);
    return parser.parse(data, actual, &fErrorCode, &fErrorLineNumber);
}
