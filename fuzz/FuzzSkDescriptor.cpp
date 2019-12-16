/*
 * Copyright 2019 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "fuzz/Fuzz.h"
#include "src/core/SkDescriptor.h"
#include "src/core/SkScalerContext.h"

DEF_FUZZ(SkDescriptor, fuzz) {
    int32_t numEntries;
    fuzz->next(&numEntries);

    // Limit this to keep the fuzz operations fast.
    if (numEntries < 0 || numEntries > 300) {
        return;
    }

    size_t len = SkDescriptor::ComputeOverhead(numEntries);
    auto desc = SkDescriptor::Alloc(len);
    for (int32_t i = 0; i<numEntries && !fuzz->exhausted(); i++) {
        uint32_t tag;
        fuzz->next(&tag);
        // Valid use of the API requires that tag is truthy and that
        // the length is aligned to 4. If the fuzzed data doesn't conform,
        // return to signal that this is "boring" data.
        if (!tag) {
            return;
        }
        size_t length;
        fuzz->next(&length);
        if (SkAlign4(length) != length) {
            return;
        }

        uint8_t choice;
        fuzz->nextRange(&choice, 0, 2);
        switch(choice) {
            case 0: { // use nullptr
                desc->addEntry(tag, length, nullptr);
                break;
            }
            case 1: { // use SkScalerContextRec
                SkScalerContextRec rec;
                fuzz->next(&rec);
                desc->addEntry(tag, sizeof(rec), &rec);
                break;
            }
            case 2: { // use arbitrary data
                if (fuzz->remaining() < length) {
                    // Can't initialize all that we requested, so bail out.
                    return;
                }
                uint8_t* bytes = new uint8_t[length];
                fuzz->nextN(bytes, length);
                desc->addEntry(tag, length, bytes);
                break;
            }
            default: {
                SK_ABORT("Did you update the range in FuzzSkDescriptor?");
            }
        }
    }

    // Exercise the API to make sure we don't step out of bounds, etc.

    desc->computeChecksum();
    desc->isValid();

    uint32_t tagToFind;
    fuzz->next(&tagToFind);

    uint32_t ignore;
    desc->findEntry(tagToFind, &ignore);
}
