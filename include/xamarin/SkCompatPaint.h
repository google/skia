/*
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCompatPaint_h
#define SkCompatPaint_h

#include "include/core/SkPaint.h"
#include "include/core/SkFont.h"
#include "include/utils/SkTextUtils.h"


class SkCompatPaint : public SkPaint {
public:
    SkCompatPaint();
    SkCompatPaint(const SkCompatPaint& paint);
    SkCompatPaint(const SkFont* font);
    ~SkCompatPaint();

public:
    void reset();

    SkFont* makeFont();

    SkFont* getFont();

    void setTextAlign(SkTextUtils::Align textAlign);
    SkTextUtils::Align getTextAlign() const;

    void setTextEncoding(SkTextEncoding encoding);
    SkTextEncoding getTextEncoding() const;

private:
    SkFont fFont;
    SkTextUtils::Align fTextAlign;
    SkTextEncoding fTextEncoding;

    typedef SkPaint INHERITED;
};


#endif
