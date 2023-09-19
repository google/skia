/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkFontMetrics.h"
#include "include/private/base/SkAssert.h"
#include "src/core/SkFontMetricsPriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

#include <optional>

void SkFontMetricsPriv::Flatten(SkWriteBuffer& buffer, const SkFontMetrics& metrics) {
    buffer.writeUInt(metrics.fFlags);
    buffer.writeScalar(metrics.fTop);
    buffer.writeScalar(metrics.fAscent);
    buffer.writeScalar(metrics.fDescent);
    buffer.writeScalar(metrics.fBottom);
    buffer.writeScalar(metrics.fLeading);
    buffer.writeScalar(metrics.fAvgCharWidth);
    buffer.writeScalar(metrics.fMaxCharWidth);
    buffer.writeScalar(metrics.fXMin);
    buffer.writeScalar(metrics.fXMax);
    buffer.writeScalar(metrics.fXHeight);
    buffer.writeScalar(metrics.fCapHeight);
    buffer.writeScalar(metrics.fUnderlineThickness);
    buffer.writeScalar(metrics.fUnderlinePosition);
    buffer.writeScalar(metrics.fStrikeoutThickness);
    buffer.writeScalar(metrics.fStrikeoutPosition);
}

std::optional<SkFontMetrics> SkFontMetricsPriv::MakeFromBuffer(SkReadBuffer& buffer) {
    SkASSERT(buffer.isValid());

    SkFontMetrics metrics;
    metrics.fFlags = buffer.readUInt();
    metrics.fTop = buffer.readScalar();
    metrics.fAscent = buffer.readScalar();
    metrics.fDescent = buffer.readScalar();
    metrics.fBottom = buffer.readScalar();
    metrics.fLeading = buffer.readScalar();
    metrics.fAvgCharWidth = buffer.readScalar();
    metrics.fMaxCharWidth = buffer.readScalar();
    metrics.fXMin = buffer.readScalar();
    metrics.fXMax = buffer.readScalar();
    metrics.fXHeight = buffer.readScalar();
    metrics.fCapHeight = buffer.readScalar();
    metrics.fUnderlineThickness = buffer.readScalar();
    metrics.fUnderlinePosition = buffer.readScalar();
    metrics.fStrikeoutThickness = buffer.readScalar();
    metrics.fStrikeoutPosition = buffer.readScalar();

    // All the reads above were valid, so return the metrics.
    if (buffer.isValid()) {
        return metrics;
    }

    return std::nullopt;
}
