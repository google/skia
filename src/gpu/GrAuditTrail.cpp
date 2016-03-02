/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAuditTrail.h"
#include "batches/GrBatch.h"

const int GrAuditTrail::kGrAuditTrailInvalidID = -1;

void GrAuditTrail::addBatch(const char* name, const SkRect& bounds) {
    SkASSERT(fEnabled);
    Batch* batch = new Batch;
    fBatchPool.emplace_back(batch);
    batch->fName = name;
    batch->fBounds = bounds;
    batch->fClientID = kGrAuditTrailInvalidID;
    batch->fBatchListID = kGrAuditTrailInvalidID;
    batch->fChildID = kGrAuditTrailInvalidID;
    
    // consume the current stack trace if any
    batch->fStackTrace = fCurrentStackTrace;
    fCurrentStackTrace.reset();
    fCurrentBatch = batch;
    
    if (fClientID != kGrAuditTrailInvalidID) {
        batch->fClientID = fClientID;
        Batches** batchesLookup = fClientIDLookup.find(fClientID);
        Batches* batches = nullptr;
        if (!batchesLookup) {
            batches = new Batches;
            fClientIDLookup.set(fClientID, batches);
        } else {
            batches = *batchesLookup;
        }

        batches->push_back(fCurrentBatch);
    }
}

void GrAuditTrail::batchingResultCombined(GrBatch* combiner) {
    int* indexPtr = fIDLookup.find(combiner);
    SkASSERT(indexPtr);
    int index = *indexPtr;
    SkASSERT(index < fBatchList.count());
    BatchNode& batch = *fBatchList[index];

    // set the ids for the child batch
    fCurrentBatch->fBatchListID = index;
    fCurrentBatch->fChildID = batch.fChildren.count();

    // Update the bounds and store a pointer to the new batch
    batch.fChildren.push_back(fCurrentBatch);
    batch.fBounds = combiner->bounds();
}

void GrAuditTrail::batchingResultNew(GrBatch* batch) {
    // Our algorithm doesn't bother to reorder inside of a BatchNode
    // so the ChildID will start at 0
    fCurrentBatch->fBatchListID = fBatchList.count();
    fCurrentBatch->fChildID = 0;

    // We use the batch pointer as a key to find the batchnode we are 'glomming' batches onto
    fIDLookup.set(batch, fCurrentBatch->fBatchListID);
    BatchNode* batchNode = new BatchNode;
    batchNode->fBounds = fCurrentBatch->fBounds;
    batchNode->fRenderTargetUniqueID = batch->renderTargetUniqueID();
    batchNode->fChildren.push_back(fCurrentBatch);
    fBatchList.emplace_back(batchNode);
}

void GrAuditTrail::getBoundsByClientID(SkTArray<BatchInfo>* outInfo, int clientID) {
    Batches** batchesLookup = fClientIDLookup.find(clientID);
    if (batchesLookup) {
        // We track which batchlistID we're currently looking at.  If it changes, then we
        // need to push back a new batch info struct.  We happen to know that batches are
        // in sequential order in the batchlist, otherwise we'd have to do more bookkeeping
        int currentBatchListID = kGrAuditTrailInvalidID;
        for (int i = 0; i < (*batchesLookup)->count(); i++) {
            const Batch* batch = (**batchesLookup)[i];

            // Because we will copy out all of the batches associated with a given
            // batch list id everytime the id changes, we only have to update our struct
            // when the id changes.
            if (kGrAuditTrailInvalidID == currentBatchListID ||
                batch->fBatchListID != currentBatchListID) {
                BatchInfo& outBatchInfo = outInfo->push_back();
                currentBatchListID = batch->fBatchListID;
                
                // copy out all of the batches so the client can display them even if
                // they have a different clientID
                const BatchNode* bn = fBatchList[currentBatchListID];
                outBatchInfo.fBounds = bn->fBounds;
                outBatchInfo.fRenderTargetUniqueID = bn->fRenderTargetUniqueID;
                for (int j = 0; j < bn->fChildren.count(); j++) {
                    BatchInfo::Batch& outBatch = outBatchInfo.fBatches.push_back();
                    const Batch* currentBatch = bn->fChildren[j];
                    outBatch.fBounds = currentBatch->fBounds;
                    outBatch.fClientID = currentBatch->fClientID;
                }
            }
        }
    }
}

void GrAuditTrail::fullReset() {
    SkASSERT(fEnabled);
    fBatchList.reset();
    fIDLookup.reset();
    // free all client batches
    fClientIDLookup.foreach([](const int&, Batches** batches) { delete *batches; });
    fClientIDLookup.reset();
    fBatchPool.reset(); // must be last, frees all of the memory
}

template <typename T>
void GrAuditTrail::JsonifyTArray(SkString* json, const char* name, const T& array,
                                 bool addComma) {
    if (array.count()) {
        if (addComma) {
            json->appendf(",");
        }
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

SkString GrAuditTrail::toJson(bool prettyPrint) const {
    SkString json;
    json.append("{");
    JsonifyTArray(&json, "Batches", fBatchList, false);
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
    Batches** batches = fClientIDLookup.find(clientID);
    if (batches) {
        JsonifyTArray(&json, "Batches", **batches, false);
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

SkString GrAuditTrail::Batch::toJson() const {
    SkString json;
    json.append("{");
    json.appendf("\"Name\": \"%s\",", fName.c_str());
    json.appendf("\"ClientID\": \"%d\",", fClientID);
    json.appendf("\"BatchListID\": \"%d\",", fBatchListID);
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

SkString GrAuditTrail::BatchNode::toJson() const {
    SkString json;
    json.append("{");
    json.appendf("\"RenderTarget\": \"%u\",", fRenderTargetUniqueID);
    skrect_to_json(&json, "Bounds", fBounds);
    JsonifyTArray(&json, "Batches", fChildren, true);
    json.append("}");
    return json;
}
