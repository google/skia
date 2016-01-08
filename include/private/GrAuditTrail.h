/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAuditTrail_DEFINED
#define GrAuditTrail_DEFINED

#include "SkString.h"
#include "SkTArray.h"

/*
 * GrAuditTrail collects a list of draw ops, detailed information about those ops, and can dump them
 * to json.
 */
class GrAuditTrail {
public:
    void addOp(SkString name) {
        fOps.push_back().fName = name;
    }

    SkString toJson() const;

    void reset() { fOps.reset(); }

private:
    struct Op {
        SkString toJson() const;
        SkString fName;
    };

    SkTArray<Op> fOps;
};

#endif
