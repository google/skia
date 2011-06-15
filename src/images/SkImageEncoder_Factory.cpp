/*
 * Copyright 2009, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "SkImageEncoder.h"
#include "SkTRegistry.h"

typedef SkTRegistry<SkImageEncoder*, SkImageEncoder::Type> EncodeReg;

// Can't use the typedef here because of complex C++ corner cases
template EncodeReg* SkTRegistry<SkImageEncoder*, SkImageEncoder::Type>::gHead;

#ifdef SK_ENABLE_LIBPNG
    extern SkImageEncoder* sk_libpng_efactory(SkImageEncoder::Type);
#endif

SkImageEncoder* SkImageEncoder::Create(Type t) {
    SkImageEncoder* codec = NULL;
    const EncodeReg* curr = EncodeReg::Head();
    while (curr) {
        if ((codec = curr->factory()(t)) != NULL) {
            return codec;
        }
        curr = curr->next();
    }
#ifdef SK_ENABLE_LIBPNG
    if ((codec = sk_libpng_efactory(t)) != NULL) {
        return codec;
    }
#endif
    return NULL;
}
