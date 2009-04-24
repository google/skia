#include "SkBitmap.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkStream.h"
#include "SkTemplates.h"

static bool decodeFile(SkBitmap* bitmap, const char srcPath[]) {
    SkFILEStream stream(srcPath);
    if (!stream.isValid()) {
        SkDebugf("ERROR: bad filename <%s>\n", srcPath);
        return false;
    }

    SkImageDecoder* codec = SkImageDecoder::Factory(&stream);
    if (NULL == codec) {
        SkDebugf("ERROR: no codec found for <%s>\n", srcPath);
        return false;
    }

    SkAutoTDelete<SkImageDecoder> ad(codec);

    stream.rewind();
    if (!codec->decode(&stream, bitmap, SkBitmap::kARGB_8888_Config,
                       SkImageDecoder::kDecodePixels_Mode)) {
        SkDebugf("ERROR: codec failed for <%s>\n", srcPath);
        return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

class SkAutoGraphics {
public:
    SkAutoGraphics() {
        SkGraphics::Init();
    }
    ~SkAutoGraphics() {
        SkGraphics::Term();
    }
};

static void show_help() {
    SkDebugf("usage: skiamge [-o out-dir] inputfiles...\n");
}

static void make_outname(SkString* dst, const char outDir[], const char src[]) {
    dst->set(outDir);
    const char* start = strrchr(src, '/');
    if (start) {
        start += 1; // skip the actual last '/'
    } else {
        start = src;
    }
    dst->append(start);
    dst->append(".png");
}

int main (int argc, char * const argv[]) {
    SkAutoGraphics ag;
    int i, outDirIndex = 0;
    SkString outDir;

    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-help")) {
            show_help();
            return 0;
        }
        if (!strcmp(argv[i], "-o")) {
            if (i == argc-1) {
                SkDebugf("ERROR: -o needs a following filename\n");
                return -1;
            }
            outDirIndex = i;
            outDir.set(argv[i+1]);
            if (outDir.c_str()[outDir.size() - 1] != '/') {
                outDir.append("/");
            }
            i += 1; // skip the out dir name
        }
    }

    for (i = 1; i < argc; i++) {
        if (i == outDirIndex) {
            i += 1; // skip this and the next entry
            continue;
        }
        
        SkBitmap bitmap;
        if (decodeFile(&bitmap, argv[i])) {
            if (outDirIndex) {
                SkString outPath;
                make_outname(&outPath, outDir.c_str(), argv[i]);
                SkDebugf("  writing %s\n", outPath.c_str());
                SkImageEncoder::EncodeFile(outPath.c_str(), bitmap,
                                           SkImageEncoder::kPNG_Type, 100);
            } else {
                SkDebugf("  decoded %s [%d %d]\n", argv[i], bitmap.width(),
                         bitmap.height());
            }
        }
    }

    return 0;
}

