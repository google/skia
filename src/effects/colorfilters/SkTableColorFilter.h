/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkTableColorFilter_DEFINED
#define SkTableColorFilter_DEFINED

#include "include/core/SkColorTable.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "src/effects/colorfilters/SkColorFilterBase.h"

class SkBitmap;
class SkReadBuffer;
class SkWriteBuffer;
struct SkStageRec;

class SkTableColorFilter final : public SkColorFilterBase {
public:
    SkTableColorFilter(sk_sp<SkColorTable> table) : fTable(table) {
        SkASSERT(fTable);
    }

    SkColorFilterBase::Type type() const override { return SkColorFilterBase::Type::kTable; }

    bool appendStages(const SkStageRec& rec, bool shaderIsOpaque) const override;

    void flatten(SkWriteBuffer& buffer) const override;

    const SkBitmap& bitmap() const { return fTable->bitmap(); }

private:
    friend void ::SkRegisterTableColorFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkTableColorFilter)

    sk_sp<SkColorTable> fTable;
};

#endif
