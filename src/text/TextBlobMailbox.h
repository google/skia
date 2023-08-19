/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sktext_TextBlobMailbox_DEFINED

#include <cstdint>

namespace sktext {
// With a Ganesh or Graphite backend, this signals the given cache it can purge
// assets related to the given blob ID. A no-op on the software backend.
void PostPurgeBlobMessage(uint32_t blobID, uint32_t cacheID);
}

#endif
