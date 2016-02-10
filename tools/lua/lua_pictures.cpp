/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "LazyDecodeBitmap.h"
#include "SkLua.h"
#include "SkLuaCanvas.h"
#include "SkPicture.h"
#include "SkCommandLineFlags.h"
#include "SkGraphics.h"
#include "SkStream.h"
#include "SkData.h"
#include "picture_utils.h"
#include "SkOSFile.h"
#include "SkImageDecoder.h"

#include <stdlib.h>

extern "C" {
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
}

static const char gStartCanvasFunc[] = "sk_scrape_startcanvas";
static const char gEndCanvasFunc[] = "sk_scrape_endcanvas";
static const char gAccumulateFunc[] = "sk_scrape_accumulate";
static const char gSummarizeFunc[] = "sk_scrape_summarize";

// Example usage for the modulo flag:
// for i in {0..5}; do lua_pictures --skpPath SKP_PATH -l YOUR_SCRIPT --modulo $i 6 &; done
DEFINE_string(modulo, "", "[--modulo <remainder> <divisor>]: only run tests for which "
              "testIndex %% divisor == remainder.");
DEFINE_string2(skpPath, r, "", "Read this .skp file or .skp files from this dir");
DEFINE_string2(luaFile, l, "", "File containing lua script to run");
DEFINE_string2(headCode, s, "", "Optional lua code to call at beginning");
DEFINE_string2(tailFunc, s, "", "Optional lua function to call at end");
DEFINE_bool2(quiet, q, false, "Silence all non-error related output");

static SkPicture* load_picture(const char path[]) {
    SkAutoTDelete<SkStream> stream(SkStream::NewFromFile(path));
    SkPicture* pic = nullptr;
    if (stream.get()) {
        pic = SkPicture::CreateFromStream(stream.get(), &sk_tools::LazyDecodeBitmap);
    }
    return pic;
}

static void call_canvas(lua_State* L, SkLuaCanvas* canvas,
                        const char pictureFile[], const char funcName[]) {
    lua_getglobal(L, funcName);
    if (!lua_isfunction(L, -1)) {
        int t = lua_type(L, -1);
        SkDebugf("--- expected %s function %d, ignoring.\n", funcName, t);
        lua_settop(L, -2);
    } else {
        canvas->pushThis();
        lua_pushstring(L, pictureFile);
        if (lua_pcall(L, 2, 0, 0) != LUA_OK) {
            SkDebugf("lua err: %s\n", lua_tostring(L, -1));
        }
    }
}

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
    SkCommandLineFlags::SetUsage("apply lua script to .skp files.");
    SkCommandLineFlags::Parse(argc, argv);

    if (FLAGS_skpPath.isEmpty()) {
        SkDebugf(".skp files or directories are required.\n");
        exit(-1);
    }
    if (FLAGS_luaFile.isEmpty()) {
        SkDebugf("missing luaFile(s)\n");
        exit(-1);
    }

    const char* summary = gSummarizeFunc;
    if (!FLAGS_tailFunc.isEmpty()) {
        summary = FLAGS_tailFunc[0];
    }

    SkAutoGraphics ag;
    SkLua L(summary);

    for (int i = 0; i < FLAGS_luaFile.count(); ++i) {
        SkAutoDataUnref data(SkData::NewFromFileName(FLAGS_luaFile[i]));
        if (nullptr == data.get()) {
            data.reset(SkData::NewEmpty());
        }
        if (!FLAGS_quiet) {
            SkDebugf("loading %s...\n", FLAGS_luaFile[i]);
        }
        if (!L.runCode(data->data(), data->size())) {
            SkDebugf("failed to load luaFile %s\n", FLAGS_luaFile[i]);
            exit(-1);
        }
    }

    if (!FLAGS_headCode.isEmpty()) {
        L.runCode(FLAGS_headCode[0]);
    }

    int moduloRemainder = -1;
    int moduloDivisor = -1;
    SkString moduloStr;

    if (FLAGS_modulo.count() == 2) {
        moduloRemainder = atoi(FLAGS_modulo[0]);
        moduloDivisor = atoi(FLAGS_modulo[1]);
        if (moduloRemainder < 0 || moduloDivisor <= 0 || moduloRemainder >= moduloDivisor) {
            SkDebugf("invalid modulo values.\n");
            return -1;
        }
    }

    for (int i = 0; i < FLAGS_skpPath.count(); i ++) {
        SkTArray<SkString> paths;
        if (sk_isdir(FLAGS_skpPath[i])) {
            // Add all .skp in this directory.
            const SkString directory(FLAGS_skpPath[i]);
            SkString filename;
            SkOSFile::Iter iter(FLAGS_skpPath[i], "skp");
            while(iter.next(&filename)) {
                paths.push_back() = SkOSPath::Join(directory.c_str(), filename.c_str());
            }
        } else {
            // Add this as an .skp itself.
            paths.push_back() = FLAGS_skpPath[i];
        }

        for (int i = 0; i < paths.count(); i++) {
            if (moduloRemainder >= 0) {
                if ((i % moduloDivisor) != moduloRemainder) {
                    continue;
                }
                moduloStr.printf("[%d.%d] ", i, moduloDivisor);
            }
            const char* path = paths[i].c_str();
            if (!FLAGS_quiet) {
                SkDebugf("scraping %s %s\n", path, moduloStr.c_str());
            }

            SkAutoTUnref<SkPicture> pic(load_picture(path));
            if (pic.get()) {
                SkAutoTUnref<SkLuaCanvas> canvas(
                                    new SkLuaCanvas(SkScalarCeilToInt(pic->cullRect().width()), 
                                                    SkScalarCeilToInt(pic->cullRect().height()),
                                                    L.get(), gAccumulateFunc));

                call_canvas(L.get(), canvas.get(), path, gStartCanvasFunc);
                canvas->drawPicture(pic);
                call_canvas(L.get(), canvas.get(), path, gEndCanvasFunc);

            } else {
                SkDebugf("failed to load\n");
            }
        }
    }
    return 0;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
