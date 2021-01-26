/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSVGDOM_DEFINED
#define SkSVGDOM_DEFINED

#include "include/core/SkFontMgr.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/private/SkTemplates.h"
#include "modules/skresources/include/SkResources.h"
#include "modules/svg/include/SkSVGIDMapper.h"

class SkCanvas;
class SkDOM;
class SkStream;
class SkSVGNode;
class SkSVGSVG;

class SkSVGDOM : public SkRefCnt {
public:
    class Builder final {
    public:
        /**
         * Specify a font manager for loading SVG fonts.
         */
        Builder& setFontManager(sk_sp<SkFontMgr>);

        /**
         * Specify a resource provider for loading images etc.
         */
        Builder& setResourceProvider(sk_sp<skresources::ResourceProvider>);

        sk_sp<SkSVGDOM> make(SkStream&) const;

    private:
        sk_sp<SkFontMgr>                     fFontMgr;
        sk_sp<skresources::ResourceProvider> fResourceProvider;
    };

    static sk_sp<SkSVGDOM> MakeFromStream(SkStream& str) {
        return Builder().make(str);
    }

    const SkSize& containerSize() const;
    void setContainerSize(const SkSize&);

    // Returns the node with the given id, or nullptr if not found.
    sk_sp<SkSVGNode>* findNodeById(const char* id);

    void render(SkCanvas*) const;

private:
    SkSVGDOM(sk_sp<SkSVGSVG>, sk_sp<SkFontMgr>, sk_sp<skresources::ResourceProvider>,
             SkSVGIDMapper&&);

    const sk_sp<SkSVGSVG>                      fRoot;
    const sk_sp<SkFontMgr>                     fFontMgr;
    const sk_sp<skresources::ResourceProvider> fResourceProvider;
    const SkSVGIDMapper                        fIDMapper;

    SkSize                 fContainerSize;
};

#endif // SkSVGDOM_DEFINED
