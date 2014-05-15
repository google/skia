/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef DumpRecord_DEFINED
#define DumpRecord_DEFINED

class SkRecord;
class SkCanvas;

/**
 * Draw the record to the supplied canvas via SkRecords::Draw, while
 * printing each draw command and run time in microseconds to stdout.
 *
 * @param timeWithCommand If true, print time next to command, else in
 *        first column.
 */
void DumpRecord(const SkRecord& record,
                SkCanvas* canvas,
                bool timeWithCommand);

#endif  // DumpRecord_DEFINED
