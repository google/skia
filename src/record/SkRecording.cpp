#include "SkRecording.h"

#include "SkRecord.h"
#include "SkRecorder.h"
#include "SkRecordDraw.h"

namespace EXPERIMENTAL {

SkPlayback::SkPlayback(const SkRecord* record) : fRecord(record) {}

SkPlayback::~SkPlayback() {
    SkDELETE(fRecord);
}

void SkPlayback::draw(SkCanvas* canvas) const {
    SkASSERT(fRecord != NULL);
    SkRecordDraw(*fRecord, canvas);
}

/*static*/ SkRecording* SkRecording::Create(int width, int height) {
    return SkNEW_ARGS(SkRecording, (width, height));
}

SkRecording::SkRecording(int width, int height) {
    SkRecord* record = SkNEW(SkRecord);
    fRecorder = SkNEW_ARGS(SkRecorder, (SkRecorder::kReadWrite_Mode, record, width, height));
    fRecord = record;
}

/*static*/ const SkPlayback* SkRecording::Delete(SkRecording* recording) {
    const SkRecord* record = recording->fRecord;
    SkDELETE(recording);
    return SkNEW_ARGS(SkPlayback, (record));
}

SkRecording::~SkRecording() {
    SkDELETE(fRecorder);
}

SkCanvas* SkRecording::canvas() {
    return fRecorder;
}

}  // namespace EXPERIMENTAL
