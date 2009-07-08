#include "gm.h"
#include "SkColorPriv.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkImageEncoder.h"

using namespace skiagm;

// need to explicitly declare this, or we get some weird infinite loop llist
template GMRegistry* GMRegistry::gHead;

class Iter {
public:
    Iter() {
        fReg = GMRegistry::Head();
    }
	
    GM* next() {
        if (fReg) {
            GMRegistry::Factory fact = fReg->factory();
            fReg = fReg->next();
            return fact(0);
        }
        return NULL;
    }
	
    static int Count() {
        const GMRegistry* reg = GMRegistry::Head();
        int count = 0;
        while (reg) {
            count += 1;
            reg = reg->next();
        }
        return count;
    }
	
private:
    const GMRegistry* fReg;
};

static SkString make_name(const char shortName[], const char configName[]) {
    SkString name(shortName);
    name.appendf("_%s", configName);
    return name;
}

static SkString make_filename(const char path[], const SkString& name) {
    SkString filename(path);
    if (filename.size() && filename[filename.size() - 1] != '/') {
        filename.append("/");
    }
    filename.appendf("%s.png", name.c_str());
    return filename;
}

/* since PNG insists on unpremultiplying our alpha, we take no precision chances
    and force all pixels to be 100% opaque, otherwise on compare we may not get
    a perfect match.
 */
static void force_all_opaque(const SkBitmap& bitmap) {
    SkAutoLockPixels lock(bitmap);
    for (int y = 0; y < bitmap.height(); y++) {
        for (int x = 0; x < bitmap.width(); x++) {
            *bitmap.getAddr32(x, y) |= (SK_A32_MASK << SK_A32_SHIFT);
        }
    }
}

static bool write_bitmap(const SkString& path, const SkBitmap& bitmap) {
    SkBitmap copy;
    bitmap.copyTo(&copy, SkBitmap::kARGB_8888_Config);
    force_all_opaque(copy);
    return SkImageEncoder::EncodeFile(path.c_str(), copy,
                                      SkImageEncoder::kPNG_Type, 100);
}

static void compare(const SkBitmap& target, const SkBitmap& base,
                    const SkString& name) {
    SkBitmap copy;
    const SkBitmap* bm = &target;
    if (target.config() != SkBitmap::kARGB_8888_Config) {
        target.copyTo(&copy, SkBitmap::kARGB_8888_Config);
        bm = &copy;
    }

    force_all_opaque(*bm);

    const int w = bm->width();
    const int h = bm->height();
    if (w != base.width() || h != base.height()) {
        SkDebugf("---- dimensions mismatch for %s base [%d %d] current [%d %d]\n",
                 name.c_str(), base.width(), base.height(), w, h);
        return;
    }

    SkAutoLockPixels bmLock(*bm);
    SkAutoLockPixels baseLock(base);

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            SkPMColor c0 = *base.getAddr32(x, y);
            SkPMColor c1 = *bm->getAddr32(x, y);
            if (c0 != c1) {
                SkDebugf("----- pixel mismatch for %s at [%d %d] base 0x%08X current 0x%08X\n",
                         name.c_str(), x, y, c0, c1);
                return;
            }
        }
    }
}

static const struct {
	SkBitmap::Config	fConfig;
	bool				fUsePicture;
	const char*			fName;
} gRec[] = {
	{ SkBitmap::kARGB_8888_Config,	false,	"8888" },
	{ SkBitmap::kARGB_4444_Config,	false,	"4444" },
	{ SkBitmap::kRGB_565_Config,	false,	"565" },
};

int main (int argc, char * const argv[]) {
    SkAutoGraphics ag;
    
    const char* writePath = NULL;   // if non-null, where we write the originals
    const char* readPath = NULL;    // if non-null, were we read from to compare

    char* const* stop = argv + argc;
    for (++argv; argv < stop; ++argv) {
        if (strcmp(*argv, "-w") == 0) {
            argv++;
            if (argv < stop && **argv) {
                writePath = *argv;
            }
        } else if (strcmp(*argv, "-r") == 0) {
            argv++;
            if (argv < stop && **argv) {
                readPath = *argv;
            }
        }
    }
    
    Iter iter;
    GM* gm;
	
    while ((gm = iter.next()) != NULL) {
		SkISize size = gm->getISize();
        SkDebugf("creating... %s [%d %d]\n", gm->shortName(),
                 size.width(), size.height());

		SkBitmap bitmap;
		for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); i++) {
			bitmap.setConfig(gRec[i].fConfig, size.width(), size.height());
			bitmap.allocPixels();
			bitmap.eraseColor(0);
			SkCanvas canvas(bitmap);

			gm->draw(&canvas);
            
            SkString name = make_name(gm->shortName(), gRec[i].fName);

            if (writePath) {
                SkString path = make_filename(writePath, name);
                bool success = write_bitmap(path, bitmap);
                if (!success) {
                    fprintf(stderr, "FAILED to write %s\n", path.c_str());
                }
            } else if (readPath) {
                SkString path = make_filename(readPath, name);
                SkBitmap orig;
                bool success = SkImageDecoder::DecodeFile(path.c_str(), &orig,
                                    SkBitmap::kARGB_8888_Config,
                                    SkImageDecoder::kDecodePixels_Mode, NULL);
                if (success) {
                    compare(bitmap, orig, name);
                } else {
                    fprintf(stderr, "FAILED to read %s\n", path.c_str());
                }
            }
		}
        SkDELETE(gm);
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

using namespace skiagm;

GM::GM() {}
GM::~GM() {}

void GM::draw(SkCanvas* canvas) {
	this->onDraw(canvas);
}


