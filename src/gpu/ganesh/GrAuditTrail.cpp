/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrAuditTrail.h"
#include "src/gpu/ganesh/ops/GrOp.h"

const int GrAuditTrail::kGrAuditTrailInvalidID = -1;

void GrAuditTrail::addOp(const GrOp* op, GrRenderTargetProxy::UniqueID proxyID) {
    SkASSERT(fEnabled);
    Op* auditOp = new Op;
    fOpPool.emplace_back(auditOp);
    auditOp->fName = op->name();
    auditOp->fBounds = op->bounds();
    auditOp->fClientID = kGrAuditTrailInvalidID;
    auditOp->fOpsTaskID = kGrAuditTrailInvalidID;
    auditOp->fChildID = kGrAuditTrailInvalidID;

    // consume the current stack trace if any
    auditOp->fStackTrace = fCurrentStackTrace;
    fCurrentStackTrace.clear();

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
    auditOp->fOpsTaskID = fOpsTask.size();
    auditOp->fChildID = 0;

    // We use the op pointer as a key to find the OpNode we are 'glomming' ops onto
    fIDLookup.set(op->uniqueID(), auditOp->fOpsTaskID);
    OpNode* opNode = new OpNode(proxyID);
    opNode->fBounds = op->bounds();
    opNode->fChildren.push_back(auditOp);
    fOpsTask.emplace_back(opNode);
}

void GrAuditTrail::opsCombined(const GrOp* consumer, const GrOp* consumed) {
    // Look up the op we are going to glom onto
    int* indexPtr = fIDLookup.find(consumer->uniqueID());
    SkASSERT(indexPtr);
    int index = *indexPtr;
    SkASSERT(index < fOpsTask.size() && fOpsTask[index]);
    OpNode& consumerOp = *fOpsTask[index];

    // Look up the op which will be glommed
    int* consumedPtr = fIDLookup.find(consumed->uniqueID());
    SkASSERT(consumedPtr);
    int consumedIndex = *consumedPtr;
    SkASSERT(consumedIndex < fOpsTask.size() && fOpsTask[consumedIndex]);
    OpNode& consumedOp = *fOpsTask[consumedIndex];

    // steal all of consumed's ops
    for (int i = 0; i < consumedOp.fChildren.size(); i++) {
        Op* childOp = consumedOp.fChildren[i];

        // set the ids for the child op
        childOp->fOpsTaskID = index;
        childOp->fChildID = consumerOp.fChildren.size();
        consumerOp.fChildren.push_back(childOp);
    }

    // Update the bounds for the combineWith node
    consumerOp.fBounds = consumer->bounds();

    // remove the old node from our opsTask and clear the combinee's lookup
    // NOTE: because we can't change the shape of the oplist, we use a sentinel
    fOpsTask[consumedIndex].reset(nullptr);
    fIDLookup.remove(consumed->uniqueID());
}

void GrAuditTrail::copyOutFromOpsTask(OpInfo* outOpInfo, int opsTaskID) {
    SkASSERT(opsTaskID < fOpsTask.size());
    const OpNode* bn = fOpsTask[opsTaskID].get();
    SkASSERT(bn);
    outOpInfo->fBounds = bn->fBounds;
    outOpInfo->fProxyUniqueID    = bn->fProxyUniqueID;
    for (int j = 0; j < bn->fChildren.size(); j++) {
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
        int currentOpsTaskID = kGrAuditTrailInvalidID;
        for (int i = 0; i < (*opsLookup)->size(); i++) {
            const Op* op = (**opsLookup)[i];

            // Because we will copy out all of the ops associated with a given op list id everytime
            // the id changes, we only have to update our struct when the id changes.
            if (kGrAuditTrailInvalidID == currentOpsTaskID || op->fOpsTaskID != currentOpsTaskID) {
                OpInfo& outOpInfo = outInfo->push_back();

                // copy out all of the ops so the client can display them even if they have a
                // different clientID
                this->copyOutFromOpsTask(&outOpInfo, op->fOpsTaskID);
            }
        }
    }
}

void GrAuditTrail::getBoundsByOpsTaskID(OpInfo* outInfo, int opsTaskID) {
    this->copyOutFromOpsTask(outInfo, opsTaskID);
}

void GrAuditTrail::fullReset() {
    SkASSERT(fEnabled);
    fOpsTask.clear();
    fIDLookup.reset();
    // free all client ops
    fClientIDLookup.foreach ([](const int&, Ops** ops) { delete *ops; });
    fClientIDLookup.reset();
    fOpPool.clear();  // must be last, frees all of the memory
}

#ifdef SK_ENABLE_DUMP_GPU
#include "src/utils/SkJSONWriter.h"

template <typename T>
void GrAuditTrail::JsonifyTArray(SkJSONWriter& writer, const char* name, const T& array) {
    if (array.size()) {
        writer.beginArray(name);
        for (int i = 0; i < array.size(); i++) {
            // Handle sentinel nullptrs
            if (array[i]) {
                array[i]->toJson(writer);
            }
        }
        writer.endArray();
    }
}

void GrAuditTrail::toJson(SkJSONWriter& writer) const {
    writer.beginObject();
    JsonifyTArray(writer, "Ops", fOpsTask);
    writer.endObject();
}

void GrAuditTrail::toJson(SkJSONWriter& writer, int clientID) const {
    writer.beginObject();
    Ops** ops = fClientIDLookup.find(clientID);
    if (ops) {
        JsonifyTArray(writer, "Ops", **ops);
    }
    writer.endObject();
}

static void skrect_to_json(SkJSONWriter& writer, const char* name, const SkRect& rect) {
    writer.beginObject(name);
    writer.appendFloat("Left", rect.fLeft);
    writer.appendFloat("Right", rect.fRight);
    writer.appendFloat("Top", rect.fTop);
    writer.appendFloat("Bottom", rect.fBottom);
    writer.endObject();
}

void GrAuditTrail::Op::toJson(SkJSONWriter& writer) const {
    writer.beginObject();
    writer.appendString("Name", fName);
    writer.appendS32("ClientID", fClientID);
    writer.appendS32("OpsTaskID", fOpsTaskID);
    writer.appendS32("ChildID", fChildID);
    skrect_to_json(writer, "Bounds", fBounds);
    if (fStackTrace.size()) {
        writer.beginArray("Stack");
        for (int i = 0; i < fStackTrace.size(); i++) {
            writer.appendString(fStackTrace[i]);
        }
        writer.endArray();
    }
    writer.endObject();
}

void GrAuditTrail::OpNode::toJson(SkJSONWriter& writer) const {
    writer.beginObject();
    writer.appendU32("ProxyID", fProxyUniqueID.asUInt());
    skrect_to_json(writer, "Bounds", fBounds);
    JsonifyTArray(writer, "Ops", fChildren);
    writer.endObject();
}
#else
template <typename T>
void GrAuditTrail::JsonifyTArray(SkJSONWriter& writer, const char* name, const T& array) {}
void GrAuditTrail::toJson(SkJSONWriter& writer) const {}
void GrAuditTrail::toJson(SkJSONWriter& writer, int clientID) const {}
void GrAuditTrail::Op::toJson(SkJSONWriter& writer) const {}
void GrAuditTrail::OpNode::toJson(SkJSONWriter& writer) const {}
#endif // SK_ENABLE_DUMP_GPU
