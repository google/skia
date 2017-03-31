/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAuditTrail.h"
#include "ops/GrOp.h"

const int GrAuditTrail::kGrAuditTrailInvalidID = -1;

void GrAuditTrail::addOp(const GrOp* op,
                         GrGpuResource::UniqueID resourceID,
                         GrRenderTargetProxy::UniqueID proxyID) {
    SkASSERT(fEnabled);
    Op* auditOp = new Op;
    fOpPool.emplace_back(auditOp);
    auditOp->fName = op->name();
    auditOp->fBounds = op->bounds();
    auditOp->fClientID = kGrAuditTrailInvalidID;
    auditOp->fOpListID = kGrAuditTrailInvalidID;
    auditOp->fChildID = kGrAuditTrailInvalidID;

    // consume the current stack trace if any
    auditOp->fStackTrace = fCurrentStackTrace;
    fCurrentStackTrace.reset();

    if (fClientID != kGrAuditTrailInvalidID) {
        auditOp->fClientID = fClientID;
        Ops** opsLookup = fClientIDLookup.find(fClientID);
        Ops* ops = nullptr;
        if (!opsLookup) {
            ops = new Ops;
            fClientIDLookup.set(fClientID, ops);
        } else {
            ops = *opsLookup;
        }

        ops->push_back(auditOp);
    }

    // Our algorithm doesn't bother to reorder inside of an OpNode so the ChildID will start at 0
    auditOp->fOpListID = fOpList.count();
    auditOp->fChildID = 0;

    // We use the op pointer as a key to find the OpNode we are 'glomming' ops onto
    fIDLookup.set(op->uniqueID(), auditOp->fOpListID);
    OpNode* opNode = new OpNode(resourceID, proxyID);
    opNode->fBounds = op->bounds();
    opNode->fChildren.push_back(auditOp);
    fOpList.emplace_back(opNode);
}

void GrAuditTrail::opsCombined(const GrOp* consumer, const GrOp* consumed) {
    // Look up the op we are going to glom onto
    int* indexPtr = fIDLookup.find(consumer->uniqueID());
    SkASSERT(indexPtr);
    int index = *indexPtr;
    SkASSERT(index < fOpList.count() && fOpList[index]);
    OpNode& consumerOp = *fOpList[index];

    // Look up the op which will be glommed
    int* consumedPtr = fIDLookup.find(consumed->uniqueID());
    SkASSERT(consumedPtr);
    int consumedIndex = *consumedPtr;
    SkASSERT(consumedIndex < fOpList.count() && fOpList[consumedIndex]);
    OpNode& consumedOp = *fOpList[consumedIndex];

    // steal all of consumed's ops
    for (int i = 0; i < consumedOp.fChildren.count(); i++) {
        Op* childOp = consumedOp.fChildren[i];

        // set the ids for the child op
        childOp->fOpListID = index;
        childOp->fChildID = consumerOp.fChildren.count();
        consumerOp.fChildren.push_back(childOp);
    }

    // Update the bounds for the combineWith node
    consumerOp.fBounds = consumer->bounds();

    // remove the old node from our opList and clear the combinee's lookup
    // NOTE: because we can't change the shape of the oplist, we use a sentinel
    fOpList[consumedIndex].reset(nullptr);
    fIDLookup.remove(consumed->uniqueID());
}

void GrAuditTrail::copyOutFromOpList(OpInfo* outOpInfo, int opListID) {
    SkASSERT(opListID < fOpList.count());
    const OpNode* bn = fOpList[opListID].get();
    SkASSERT(bn);
    outOpInfo->fBounds = bn->fBounds;
    outOpInfo->fResourceUniqueID = bn->fResourceUniqueID;
    outOpInfo->fProxyUniqueID    = bn->fProxyUniqueID;
    for (int j = 0; j < bn->fChildren.count(); j++) {
        OpInfo::Op& outOp = outOpInfo->fOps.push_back();
        const Op* currentOp = bn->fChildren[j];
        outOp.fBounds = currentOp->fBounds;
        outOp.fClientID = currentOp->fClientID;
    }
}

void GrAuditTrail::getBoundsByClientID(SkTArray<OpInfo>* outInfo, int clientID) {
    Ops** opsLookup = fClientIDLookup.find(clientID);
    if (opsLookup) {
        // We track which oplistID we're currently looking at.  If it changes, then we need to push
        // back a new op info struct.  We happen to know that ops are in sequential order in the
        // oplist, otherwise we'd have to do more bookkeeping
        int currentOpListID = kGrAuditTrailInvalidID;
        for (int i = 0; i < (*opsLookup)->count(); i++) {
            const Op* op = (**opsLookup)[i];

            // Because we will copy out all of the ops associated with a given op list id everytime
            // the id changes, we only have to update our struct when the id changes.
            if (kGrAuditTrailInvalidID == currentOpListID || op->fOpListID != currentOpListID) {
                OpInfo& outOpInfo = outInfo->push_back();

                // copy out all of the ops so the client can display them even if they have a
                // different clientID
                this->copyOutFromOpList(&outOpInfo, op->fOpListID);
            }
        }
    }
}

void GrAuditTrail::getBoundsByOpListID(OpInfo* outInfo, int opListID) {
    this->copyOutFromOpList(outInfo, opListID);
}

void GrAuditTrail::fullReset() {
    SkASSERT(fEnabled);
    fOpList.reset();
    fIDLookup.reset();
    // free all client ops
    fClientIDLookup.foreach ([](const int&, Ops** ops) { delete *ops; });
    fClientIDLookup.reset();
    fOpPool.reset();  // must be last, frees all of the memory
}

template <typename T>
void GrAuditTrail::JsonifyTArray(SkString* json, const char* name, const T& array,
                                 bool addComma) {
    if (array.count()) {
        if (addComma) {
            json->appendf(",");
        }
        json->appendf("\"%s\": [", name);
        const char* separator = "";
        for (int i = 0; i < array.count(); i++) {
            // Handle sentinel nullptrs
            if (array[i]) {
                json->appendf("%s", separator);
                json->append(array[i]->toJson());
                separator = ",";
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

SkString GrAuditTrail::toJson(bool prettyPrint) const {
    SkString json;
    json.append("{");
    JsonifyTArray(&json, "Ops", fOpList, false);
    json.append("}");

    if (prettyPrint) {
        return pretty_print_json(json);
    } else {
        return json;
    }
}

SkString GrAuditTrail::toJson(int clientID, bool prettyPrint) const {
    SkString json;
    json.append("{");
    Ops** ops = fClientIDLookup.find(clientID);
    if (ops) {
        JsonifyTArray(&json, "Ops", **ops, false);
    }
    json.appendf("}");

    if (prettyPrint) {
        return pretty_print_json(json);
    } else {
        return json;
    }
}

static void skrect_to_json(SkString* json, const char* name, const SkRect& rect) {
    json->appendf("\"%s\": {", name);
    json->appendf("\"Left\": %f,", rect.fLeft);
    json->appendf("\"Right\": %f,", rect.fRight);
    json->appendf("\"Top\": %f,", rect.fTop);
    json->appendf("\"Bottom\": %f", rect.fBottom);
    json->append("}");
}

SkString GrAuditTrail::Op::toJson() const {
    SkString json;
    json.append("{");
    json.appendf("\"Name\": \"%s\",", fName.c_str());
    json.appendf("\"ClientID\": \"%d\",", fClientID);
    json.appendf("\"OpListID\": \"%d\",", fOpListID);
    json.appendf("\"ChildID\": \"%d\",", fChildID);
    skrect_to_json(&json, "Bounds", fBounds);
    if (fStackTrace.count()) {
        json.append(",\"Stack\": [");
        for (int i = 0; i < fStackTrace.count(); i++) {
            json.appendf("\"%s\"", fStackTrace[i].c_str());
            if (i < fStackTrace.count() - 1) {
                json.append(",");
            }
        }
        json.append("]");
    }
    json.append("}");
    return json;
}

SkString GrAuditTrail::OpNode::toJson() const {
    SkString json;
    json.append("{");
    json.appendf("\"ResourceID\": \"%u\",", fResourceUniqueID.asUInt());
    json.appendf("\"ProxyID\": \"%u\",", fProxyUniqueID.asUInt());
    skrect_to_json(&json, "Bounds", fBounds);
    JsonifyTArray(&json, "Ops", fChildren, true);
    json.append("}");
    return json;
}
