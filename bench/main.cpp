#include "SkCanvas.h"
#include "SkImageEncoder.h"
#include "SkString.h"
#include "SkTime.h"
#include "SkTRegistry.h"

#include "SkBenchmark.h"

typedef SkTRegistry<SkBenchmark*, void*> BenchRegistry;

class Iter {
public:
    Iter() {
        fBench = BenchRegistry::Head();
    }
    
    SkBenchmark* next() {
        if (fBench) {
            BenchRegistry::Factory f = fBench->factory();
            fBench = fBench->next();
            return f(0);
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

static void saveFile(const char name[], const char config[], const char dir[],
                     const SkBitmap& bm) {
    SkBitmap copy;
    if (!bm.copyTo(&copy, SkBitmap::kARGB_8888_Config)) {
        return;
    }

    SkString str;
    make_filename(name, &str);
    str.appendf("_%s.png", config);
    str.prepend(dir);
    ::remove(str.c_str());
    SkImageEncoder::EncodeFile(str.c_str(), copy, SkImageEncoder::kPNG_Type,
                               100);
}

static void performClip(SkCanvas* canvas, int w, int h) {
    SkRect r;
    
    r.set(SkIntToScalar(10), SkIntToScalar(10),
          SkIntToScalar(w*2/3), SkIntToScalar(h*2/3));
    canvas->clipRect(r, SkRegion::kIntersect_Op);

    r.set(SkIntToScalar(w/3), SkIntToScalar(h/3),
          SkIntToScalar(w-10), SkIntToScalar(h-10));
    canvas->clipRect(r, SkRegion::kXOR_Op);
}

static void performRotate(SkCanvas* canvas, int w, int h) {
    const SkScalar x = SkIntToScalar(w) / 2;
    const SkScalar y = SkIntToScalar(h) / 2;
    
    canvas->translate(x, y);
    canvas->rotate(SkIntToScalar(35));
    canvas->translate(-x, -y);
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

static int findConfig(const char config[]) {
    for (size_t i = 0; i < SK_ARRAY_COUNT(gConfigs); i++) {
        if (!strcmp(config, gConfigs[i].fName)) {
            return i;
        }
    }
    return -1;
}

int main (int argc, char * const argv[]) {
    int repeatDraw = 1;
    int forceAlpha = 0xFF;
    bool forceAA = true;
    bool doRotate = false;
    bool doClip = false;

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
        } else if (!strcmp(*argv, "-rotate")) {
            doRotate = true;
        } else if (!strcmp(*argv, "-clip")) {
            doClip = true;
        } else if (strcmp(*argv, "-forceAA") == 0) {
            forceAA = true;
        } else if (strcmp(*argv, "-forceBW") == 0) {
            forceAA = false;
        } else if (strcmp(*argv, "-forceBlend") == 0) {
            forceAlpha = 0x80;
        } else if (strcmp(*argv, "-forceOpaque") == 0) {
            forceAlpha = 0xFF;
        } else {
            int index = findConfig(*argv);
            if (index >= 0) {
                outConfig = gConfigs[index].fConfig;
            }
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

        printf("running bench %8s", bench->getName());

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
            
            if (doClip) {
                performClip(&canvas, dim.fX, dim.fY);
            }
            if (doRotate) {
                performRotate(&canvas, dim.fX, dim.fY);
            }

            SkMSec now = SkTime::GetMSecs();
            for (int i = 0; i < repeatDraw; i++) {
                SkAutoCanvasRestore acr(&canvas, true);
                bench->draw(&canvas);
            }
            if (repeatDraw > 1) {
                printf("  %4s:%7.2f", configName,
                       (SkTime::GetMSecs() - now) / (double)repeatDraw);
            }
            if (outDir.size() > 0) {
                saveFile(bench->getName(), configName, outDir.c_str(), bm);
            }
        }
        printf("\n");
    }
    
    return 0;
}
