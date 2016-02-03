/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAuditTrail.h"

void GrAuditTrail::JsonifyTArray(SkString* json, const char* name, const FrameArray& array) {
    if (array.count()) {
        json->appendf("\"%s\": [", name);
        for (int i = 0; i < array.count(); i++) {
            json->append(array[i]->toJson());
            if (i < array.count() - 1) {
                json->append(",");
            }
        }
        json->append("]");
    }
}

// This will pretty print a very small subset of json
// The parsing rules are straightforward, aside from the fact that we do not want an extra newline
// before ',' and after '}', so we have a comma exception rule.
class PrettyPrintJson {
public:
    SkString prettify(const SkString& json) {
        fPrettyJson.reset();
        fTabCount = 0;
        fFreshLine = false;
        fCommaException = false;
        for (size_t i = 0; i < json.size(); i++) {
            if ('[' == json[i] || '{' == json[i]) {
                this->newline();
                this->appendChar(json[i]);
                fTabCount++;
                this->newline();
            } else if (']' == json[i] || '}' == json[i]) {
                fTabCount--;
                this->newline();
                this->appendChar(json[i]);
                fCommaException = true;
            } else if (',' == json[i]) {
                this->appendChar(json[i]);
                this->newline();
            } else {
                this->appendChar(json[i]);
            }
        }
        return fPrettyJson;
    }
private:
    void appendChar(char appendee) {
        if (fCommaException && ',' != appendee) {
            this->newline();
        }
        this->tab();
        fPrettyJson += appendee;
        fFreshLine = false;
        fCommaException = false;
    }

    void tab() {
        if (fFreshLine) {
            for (int i = 0; i < fTabCount; i++) {
                fPrettyJson += '\t';
            }
        }
    }

    void newline() {
        if (!fFreshLine) {
            fFreshLine = true;
            fPrettyJson += '\n';
        }
    }

    SkString fPrettyJson;
    int fTabCount;
    bool fFreshLine;
    bool fCommaException;
};

static SkString pretty_print_json(SkString json) {
    class PrettyPrintJson prettyPrintJson;
    return prettyPrintJson.prettify(json);
}

SkString GrAuditTrail::toJson() const {
    SkString json;
    json.append("{");
    JsonifyTArray(&json, "Stacks", fFrames);
    json.append("}");

    // TODO if this becomes a performance issue we should make pretty print configurable
    return pretty_print_json(json);
}

SkString GrAuditTrail::Frame::toJson() const {
    SkString json;
    json.append("{");
    json.appendf("\"Name\": \"%s\",", fName);
    JsonifyTArray(&json, "Frames", fChildren);
    json.append("}");
    return json;
}

SkString GrAuditTrail::Batch::toJson() const {
    SkString json;
    json.append("{");
    json.appendf("\"Name\": \"%s\",", fName);
    json.append("\"Bounds\": {");
    json.appendf("\"Left\": %f,", fBounds.fLeft);
    json.appendf("\"Right\": %f,", fBounds.fRight);
    json.appendf("\"Top\": %f,", fBounds.fTop);
    json.appendf("\"Bottom\": %f", fBounds.fBottom);
    json.append("}");
    json.append("}");
    return json;
}
