//#include <iostream>
#include "SkCanvas.h"
#include "SkImageEncoder.h"
#include "SkString.h"
#include "SkTime.h"

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

static const struct {
    SkBitmap::Config    fConfig;
    const char*         fName;
} gConfigs[] = {
    { SkBitmap::kARGB_8888_Config,  "8888" },
    { SkBitmap::kRGB_565_Config,    "565",  },
    { SkBitmap::kARGB_4444_Config,  "4444", },
    { SkBitmap::kA8_Config,         "A8",   }
};

int main (int argc, char * const argv[]) {
    int repeatDraw = 1;
    int forceAlpha = 0xFF;
    bool forceAA = true;

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
        } else if (strcmp(*argv, "-repeat") == 0) {
            argv++;
            if (argv < stop) {
                repeatDraw = atoi(*argv);
                if (repeatDraw < 1) {
                    repeatDraw = 1;
                }
            } else {
                fprintf(stderr, "missing arg for -repeat\n");
                return -1;
            }
        } else if (strcmp(*argv, "-forceAA") == 0) {
            forceAA = true;
        } else if (strcmp(*argv, "-forceBW") == 0) {
            forceAA = false;
        } else if (strcmp(*argv, "-forceBlend") == 0) {
            forceAlpha = 0x80;
        } else if (strcmp(*argv, "-forceOpaque") == 0) {
            forceAlpha = 0xFF;
        }
    }
    
    const char* configName = "";
    int configCount = SK_ARRAY_COUNT(gConfigs);
    
    Iter iter;
    SkBenchmark* bench;
    while ((bench = iter.next()) != NULL) {
        SkIPoint dim = bench->getSize();
        if (dim.fX <= 0 || dim.fY <= 0) {
            continue;
        }
        
        bench->setForceAlpha(forceAlpha);
        bench->setForceAA(forceAA);

        printf("running bench %16s", bench->getName());

        for (int configIndex = 0; configIndex < configCount; configIndex++) {
            if (configCount > 1) {
                outConfig = gConfigs[configIndex].fConfig;
                configName = gConfigs[configIndex].fName;
            }
            
            SkBitmap bm;
            bm.setConfig(outConfig, dim.fX, dim.fY);
            bm.allocPixels();
            
            SkCanvas canvas(bm);
            canvas.drawColor(SK_ColorWHITE);
            
            SkMSec now = SkTime::GetMSecs();
            for (int i = 0; i < repeatDraw; i++) {
                bench->draw(&canvas);
            }
            if (repeatDraw > 1) {
                printf("  %4s:%7.2f", configName,
                       (SkTime::GetMSecs() - now) / (double)repeatDraw);
            }
        }
        printf("\n");

#if 0        
        SkString str;
        make_filename(bench->getName(), &str);
        str.prepend(outDir);
        str.append(".png");
        ::remove(str.c_str());
        SkImageEncoder::EncodeFile(str.c_str(), bm, SkImageEncoder::kPNG_Type,
                                   100);
#endif
    }
    
    return 0;
}
