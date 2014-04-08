#include "SkRecordCulling.h"

#include "SkRecords.h"
#include "SkTDArray.h"

namespace {

struct Annotator {
    unsigned index;
    SkTDArray<SkRecords::PushCull*> pushStack;

    // Do nothing to most record types.
    template <typename T> void operator()(T*) {}
};

template <> void Annotator::operator()(SkRecords::PushCull* push) {
    // Store the push's index for now.  We'll calculate the offset using this in the paired pop.
    push->popOffset = index;
    pushStack.push(push);
}

template <> void Annotator::operator()(SkRecords::PopCull* pop) {
    SkRecords::PushCull* push = pushStack.top();
    pushStack.pop();

    SkASSERT(index > push->popOffset);          // push->popOffset holds the index of the push.
    push->popOffset = index - push->popOffset;  // Now it's the offset between push and pop.
}

}  // namespace

void SkRecordAnnotateCullingPairs(SkRecord* record) {
    Annotator annotator;

    for (annotator.index = 0; annotator.index < record->count(); annotator.index++) {
        record->mutate(annotator.index, annotator);
    }
}
