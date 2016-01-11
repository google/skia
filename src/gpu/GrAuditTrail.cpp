/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAuditTrail.h"

SkString GrAuditTrail::toJson() const {
    SkString json;
    json.append("{\n");
    json.append("\"Ops\": [\n");
    for (int i = 0; i < fOps.count(); i++) {
        json.append(fOps[i].toJson());
        if (i < fOps.count() - 1) {
            json.append(",\n");
        }
    }
    json.append("]\n");
    json.append("}\n");
    return json;
}

SkString GrAuditTrail::Op::toJson() const {
    SkString json;
    json.append("{\n");
    json.appendf("\"Name\": \"%s\"\n", fName.c_str());
    json.append("}\n");
    return json;
}

