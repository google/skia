/* libs/graphics/svg/SkSVGImage.cpp
**
** Copyright 2006, The Android Open Source Project
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

#include "SkSVGImage.h"
#include "SkSVGParser.h"

const SkSVGAttribute SkSVGImage::gAttributes[] = {
    SVG_ATTRIBUTE(height),
    SVG_ATTRIBUTE(width),
    SVG_ATTRIBUTE(x),
    SVG_LITERAL_ATTRIBUTE(xlink:href, f_xlink_href),
    SVG_ATTRIBUTE(y)
};

DEFINE_SVG_INFO(Image)

void SkSVGImage::translate(SkSVGParser& parser, bool defState) {
    parser._startElement("image");
    INHERITED::translate(parser, defState);
    SVG_ADD_ATTRIBUTE(x);
    SVG_ADD_ATTRIBUTE(y);
//  SVG_ADD_ATTRIBUTE(width);
//  SVG_ADD_ATTRIBUTE(height);
    translateImage(parser);
    parser._endElement();
}

void SkSVGImage::translateImage(SkSVGParser& parser) {
    SkASSERT(f_xlink_href.size() > 0);
    const char* data = f_xlink_href.c_str();
    SkASSERT(strncmp(data, "data:image/", 11) == 0);
    data += 11;
    SkASSERT(strncmp(data, "png;", 4) == 0 || strncmp(data, "jpeg;", 5) == 0);
    data = strchr(data, ';');
    SkASSERT(strncmp(data, ";base64,", 8) == 0);
    data += 8;
    parser._addAttribute("base64", data);
}
