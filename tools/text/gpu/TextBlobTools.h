/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sktext_gpu_TextBlobPriv_DEFINED
#define sktext_gpu_TextBlobPriv_DEFINED

namespace sktext::gpu {
class AtlasSubRun;
class TextBlob;

class TextBlobTools final {
public:
    static const AtlasSubRun* FirstSubRun(const TextBlob*);

private:
    TextBlobTools();
};

}  // namespace sktext::gpu

#endif
