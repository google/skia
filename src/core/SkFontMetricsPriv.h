/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontMetricsPriv_DEFINED
#define SkFontMetricsPriv_DEFINED

#include <optional>

class SkReadBuffer;
class SkWriteBuffer;
struct SkFontMetrics;

class SkFontMetricsPriv {
public:
    static void Flatten(SkWriteBuffer& buffer, const SkFontMetrics& metrics);
    static std::optional<SkFontMetrics> MakeFromBuffer(SkReadBuffer& buffer);
};
#endif //SkFontMetricsPriv_DEFINED
