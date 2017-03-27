/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"
#include "SkCanvas.h"
#include "SkCommandLineFlags.h"
#include "SkOSPath.h"
#include "SkPath.h"
#include "SkPicture.h"
#include "SkStream.h"
#include <stack>

DEFINE_string(pathfinderTrail, "", "List of keystrokes to execute upon loading a pathfinder.");

/**
 * This is a simple utility designed to extract the paths from an SKP file and then isolate a single
 * one of them. Use the 'x' and 'X' keys to guide a binary search:
 *
 *   'x': Throw out half the paths.
 *   'X': Toggle which half gets tossed and which half is kept.
 *   'Z': Back up one level.
 *   'D': Dump the path.
 */
class PathFinderView : public SampleView, public SkCanvas {
public:
    PathFinderView(const char name[] = nullptr)
        : SkCanvas(4096, 4096, nullptr)
        , fFilename(name) {
        SkFILEStream stream(fFilename.c_str());
        if (!stream.isValid()) {
            SkDebugf("couldn't load picture at \"%s\"\n", fFilename.c_str());
            return;
        }
        sk_sp<SkPicture> pic = SkPicture::MakeFromStream(&stream);
        if (!pic) {
            SkDebugf("couldn't load picture at \"%s\"\n", fFilename.c_str());
            return;
        }
        pic->playback(this);
        for (int i = 0; i < FLAGS_pathfinderTrail.count(); ++i) {
            const char* key = FLAGS_pathfinderTrail[i];
            while (*key) {
                this->handleKeystroke(*key++);
            }
        }
    }

    ~PathFinderView() override {}

private:
    // Called through SkPicture::playback during construction.
    void onDrawPath(const SkPath& path, const SkPaint& paint) override {
        fPaths.push_back() = {path, paint, this->getTotalMatrix()};
    }

    // overrides from SkEventSink
    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SkString name("PATHFINDER:");
            const char* basename = strrchr(fFilename.c_str(), SkOSPath::SEPARATOR);
            name.append(basename ? basename+1: fFilename.c_str());
            SampleCode::TitleR(evt, name.c_str());
            return true;
        }
        SkUnichar key;
        if (SampleCode::CharQ(*evt, &key)) {
            if (this->handleKeystroke(key)) {
                return true;
            }
        }
        return this->INHERITED::onQuery(evt);
    }

    bool handleKeystroke(SkUnichar key) {
        switch (key) {
            case 'X':
                if (!fTossedPaths.empty()) {
                    SkTSwap(fPaths, fTossedPaths);
                    if ('X' == fTrail.back()) {
                        fTrail.pop_back();
                    } else {
                        fTrail.push_back('X');
                    }
                    this->inval(nullptr);
                }
                return true;
            case 'x':
                if (fPaths.count() > 1) {
                    int midpt = (fPaths.count() + 1) / 2;
                    fPathHistory.emplace(fPaths, fTossedPaths);
                    fTossedPaths.reset(fPaths.begin() + midpt, fPaths.count() - midpt);
                    fPaths.resize_back(midpt);
                    fTrail.push_back('x');
                    this->inval(nullptr);
                }
                return true;
            case 'Z': {
                if (!fPathHistory.empty()) {
                    fPaths = fPathHistory.top().first;
                    fTossedPaths = fPathHistory.top().second;
                    fPathHistory.pop();
                    char ch;
                    do {
                        ch = fTrail.back();
                        fTrail.pop_back();
                    } while (ch != 'x');
                    this->inval(nullptr);
                }
                return true;
            }
            case 'D':
                SkDebugf("SampleApp --pathfinder %s", fFilename.c_str());
                if (!fTrail.empty()) {
                    SkDebugf(" --pathfinderTrail ", fFilename.c_str());
                    for (char ch : fTrail) {
                        SkDebugf("%c", ch);
                    }
                }
                SkDebugf("\n");
                for (const FoundPath& foundPath : fPaths) {
                    foundPath.fPath.dump();
                }
                return true;
        }
        return false;
    }

    void onDrawContent(SkCanvas* canvas) override {
        for (const FoundPath& path : fPaths) {
            SkAutoCanvasRestore acr(canvas, true);
            canvas->concat(path.fViewMatrix);
            canvas->drawPath(path.fPath, path.fPaint);
        }
    }

    struct FoundPath {
        SkPath     fPath;
        SkPaint    fPaint;
        SkMatrix   fViewMatrix;
    };

    SkString              fFilename;
    SkTArray<FoundPath>   fPaths;
    SkTArray<FoundPath>   fTossedPaths;
    SkTArray<char>        fTrail;

    std::stack<std::pair<SkTArray<FoundPath>, SkTArray<FoundPath>>> fPathHistory;

    typedef SampleView INHERITED;
};

SampleView* CreateSamplePathFinderView(const char filename[]) {
    return new PathFinderView(filename);
}
