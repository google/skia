//#include <iostream>
#include "SkCanvas.h"
#include "SkImageEncoder.h"
#include "SkString.h"

#include "SkBenchmark.h"

typedef SkTRegistry<SkBenchmark> BenchRegistry;

class Iter {
public:
    Iter() {
        fBench = BenchRegistry::Head();
    }
    
    SkBenchmark* next() {
        if (fBench) {
            BenchRegistry::Factory f = fBench->factory();
            fBench = fBench->next();
            return f();
        }
        return NULL;
    }
    
private:
    const BenchRegistry* fBench;
};

static void make_filename(const char name[], SkString* path) {
    path->set(name);
    for (int i = 0; name[i]; i++) {
        switch (name[i]) {
            case '/':
            case '\\':
            case ' ':
            case ':':
                path->writable_str()[i] = '-';
                break;
            default:
                break;
        }
    }
}

int main (int argc, char * const argv[]) {
    SkString outDir;
    SkBitmap::Config outConfig = SkBitmap::kARGB_8888_Config;

    char* const* stop = argv + argc;
    for (++argv; argv < stop; ++argv) {
        if (strcmp(*argv, "-o") == 0) {
            argv++;
            if (argv < stop && **argv) {
                outDir.set(*argv);
                if (outDir.c_str()[outDir.size() - 1] != '/') {
                    outDir.append("/");
                }
            }
        } else if (strcmp(*argv, "-8888") == 0) {
            outConfig = SkBitmap::kARGB_8888_Config;
        } else if (strcmp(*argv, "-565") == 0) {
            outConfig = SkBitmap::kRGB_565_Config;
        } else if (strcmp(*argv, "-4444") == 0) {
            outConfig = SkBitmap::kARGB_4444_Config;
        } else if (strcmp(*argv, "-a8") == 0) {
            outConfig = SkBitmap::kA8_Config;
        }
    }

    Iter iter;
    SkBenchmark* bench;
    while ((bench = iter.next()) != NULL) {
        SkIPoint dim = bench->getSize();
        if (dim.fX <= 0 || dim.fY <= 0) {
            continue;
        }

        SkBitmap bm;
        bm.setConfig(outConfig, dim.fX, dim.fY);
        bm.allocPixels();
        
        SkCanvas canvas(bm);
        canvas.drawColor(SK_ColorWHITE);
        bench->draw(&canvas);
        
        SkString str;
        make_filename(bench->getName(), &str);
        str.prepend(outDir);
        str.append(".png");
        ::remove(str.c_str());
        SkImageEncoder::EncodeFile(str.c_str(), bm, SkImageEncoder::kPNG_Type,
                                   100);
    }
    
    return 0;
}
