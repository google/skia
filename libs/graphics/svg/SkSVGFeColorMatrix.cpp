/* libs/graphics/svg/SkSVGFeColorMatrix.cpp
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

#include "SkSVGFeColorMatrix.h"
#include "SkSVGParser.h"

const SkSVGAttribute SkSVGFeColorMatrix::gAttributes[] = {
    SVG_LITERAL_ATTRIBUTE(color-interpolation-filters, f_color_interpolation_filters),
    SVG_ATTRIBUTE(result),
    SVG_ATTRIBUTE(type),
    SVG_ATTRIBUTE(values)
};

DEFINE_SVG_INFO(FeColorMatrix)

void SkSVGFeColorMatrix::translate(SkSVGParser& parser, bool defState) {
    INHERITED::translate(parser, defState);
}
