/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkForceLinking.h"
#include "SkGraphics.h"
#include "SkOSFile.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"
#include "SkStream.h"
#include "SkTemplates.h"
#include "flags/SkCommandLineFlags.h"
#include "gm.h"

DEFINE_string2(match,
               m,
               NULL,
               "[~][^]substring[$] [...] of GM name to run.\n"
               "Multiple matches may be separated by spaces.\n"
               "~ causes a matching GM to always be skipped\n"
               "^ requires the start of the GM to match\n"
               "$ requires the end of the GM to match\n"
               "^ and $ requires an exact match\n"
               "If a GM does not match any list entry,\n"
               "it is skipped unless some list entry starts with ~");

DEFINE_string2(writePath, w, "", "Write output in this directory as .skps.");

__SK_FORCE_IMAGE_DECODER_LINKING;

static void gmtoskp(skiagm::GM* gm, SkWStream* outputStream) {
    SkPictureRecorder pictureRecorder;
    SkRect bounds = SkRect::MakeWH(SkIntToScalar(gm->getISize().width()),
                                   SkIntToScalar(gm->getISize().height()));
    SkCanvas* canvas = pictureRecorder.beginRecording(bounds, NULL, 0);
    canvas->concat(gm->getInitialTransform());
    gm->draw(canvas);
    canvas->flush();
    SkAutoTUnref<SkPicture> pict(pictureRecorder.endRecordingAsPicture());
    pict->serialize(outputStream, NULL);
}

int main(int argc, char** argv) {
    SkAutoGraphics ag;
    SkCommandLineFlags::SetUsage("");
    SkCommandLineFlags::Parse(argc, argv);
    if (FLAGS_writePath.isEmpty()) {
        SkDebugf("You need to specify a --writePath option");
        return 1;
    }
    const char* writePath = FLAGS_writePath[0];
    if (!sk_mkdir(writePath)) {
        
        return 2;
    }
    for (const skiagm::GMRegistry* reg = skiagm::GMRegistry::Head();
         reg != NULL;
         reg = reg->next()) {
        SkAutoTDelete<skiagm::GM> gm(reg->factory()(NULL));
        if (!gm.get()) {
            continue;
        }
        const char* name = gm->getName();
        if (!SkCommandLineFlags::ShouldSkip(FLAGS_match, name)) {
            SkString path = SkOSPath::Join(writePath, name);
            path.append(".skp");
            SkFILEWStream outputStream(path.c_str());
            if (!outputStream.isValid()) {
                SkDebugf("Could not open file %s\n", path.c_str());
                return 3;
            }
            gmtoskp(gm, &outputStream);
        }
    }
    return 0;
}
