/* include/graphics/SkTypeface.h
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

#ifndef SkTypeface_DEFINED
#define SkTypeface_DEFINED

#include "SkRefCnt.h"

/** \class SkTypeface

    The SkTypeface class specifies the typeface and intrinsic style of a font.
    This is used in the paint, along with optionally algorithmic settings like
    textSize, textSkewX, textScaleX, kFakeBoldText_Mask, to specify
    how text appears when drawn (and measured).
*/
class SkTypeface : public SkRefCnt {
public:
    /** Style enum specifies the possible intrinsic style attributes of a given typeface
    */
    enum Style {
        kNormal = 0,
        kBold   = 0x01,
        kItalic = 0x02,

        // helpers
        kBoldItalic = 0x03
    };

    /** Returns the typeface's intrinsic style attributes
    */
    Style   getStyle() const { return (Style)fStyle; }
    /** Returns true if getStyle() has the kBold bit set.
    */
    bool    isBold() const { return (fStyle & kBold) != 0; }
    /** Returns true if getStyle() has the kItalic bit set.
    */
    bool    isItalic() const { return (fStyle & kItalic) != 0; }
    
    /** Create a typeface object given a family name, and option style information.
        If NULL is passed for the name, then the "default" font will be chosen.
        The resulting typeface object can be queried (getStyle()) to discover what
        its "real" style characteristics are.
        
        @param familyName  May be NULL. The name of the font family.
        @param s           The style (normal, bold, italic) of the type face.

    */
    static SkTypeface* Create(const char familyName[], Style s = kNormal);
    /** Create a typeface object that best matches the specified existing typeface
        and the specified Style. Use this call if you want to pick a new style
        from the same family of an existing typeface object. If family is NULL,
        this selects from the default font's family.
        
        @param family  May be NULL. The name of the existing type face.
        @param s       The style (normal, bold, italic) of the type face.
    */
    static SkTypeface* CreateFromTypeface(const SkTypeface* family, Style s);

protected:
    void setStyle(Style s) {
        fStyle = SkToU8(s);
    }
    
    SkTypeface() {}

private:
    uint8_t fStyle;
};

#endif
