/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSurfaceCharacterization.h"

#if SK_SUPPORT_GPU
bool SkSurfaceCharacterization::operator==(const SkSurfaceCharacterization& other) const {
    if (!this->isValid() || !other.isValid()) {
        return false;
    }

    if (fContextInfo->fContextUniqueID != other.fContextInfo->fContextUniqueID) {
        return false;
    }

    return fCacheMaxResourceBytes == other.fCacheMaxResourceBytes &&
           fOrigin == other.fOrigin &&
           fImageInfo == other.fImageInfo &&
           fConfig == other.fConfig &&
           fFSAAType == other.fFSAAType &&
           fStencilCnt == other.fStencilCnt &&
           fIsTextureable == other.fIsTextureable &&
           fIsMipMapped == other.fIsMipMapped &&
           fSurfaceProps == other.fSurfaceProps;
}

#endif
