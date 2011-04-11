#include "gm.h"
#include "SkColorPriv.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkStream.h"
#include "SkRefCnt.h"

#include "GrContext.h"
#include "SkGpuCanvas.h"
#include "SkGpuDevice.h"
#include "SkEGLContext.h"
#include "SkDevice.h"

#ifdef SK_SUPPORT_PDF
	#include "SkPDFDevice.h"
	#include "SkPDFDocument.h"
#endif

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

static SkString make_filename(const char path[], const SkString& name, const char suffix[]) {
    SkString filename(path);
    if (filename.size() && filename[filename.size() - 1] != '/') {
        filename.append("/");
    }
    filename.appendf("%s.%s", name.c_str(), suffix);
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

static inline SkPMColor compute_diff_pmcolor(SkPMColor c0, SkPMColor c1) {
	int dr = SkGetPackedR32(c0) - SkGetPackedR32(c1);
	int dg = SkGetPackedG32(c0) - SkGetPackedG32(c1);
	int db = SkGetPackedB32(c0) - SkGetPackedB32(c1);
	return SkPackARGB32(0xFF, SkAbs32(dr), SkAbs32(dg), SkAbs32(db));
}

static void compute_diff(const SkBitmap& target, const SkBitmap& base,
						 SkBitmap* diff) {
	SkAutoLockPixels alp(*diff);

    const int w = target.width();
    const int h = target.height();
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            SkPMColor c0 = *base.getAddr32(x, y);
            SkPMColor c1 = *target.getAddr32(x, y);
			SkPMColor d = 0;
			if (c0 != c1) {
				d = compute_diff_pmcolor(c0, c1);
			}
			*diff->getAddr32(x, y) = d;
		}
	}
}

static bool compare(const SkBitmap& target, const SkBitmap& base,
                    const SkString& name, SkBitmap* diff) {
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
        return false;
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

				if (diff) {
					diff->setConfig(SkBitmap::kARGB_8888_Config, w, h);
					diff->allocPixels();
					compute_diff(*bm, base, diff);
				}
                return false;
            }
        }
    }

	// they're equal
	return true;
}

static bool write_pdf(const SkString& path, const SkDynamicMemoryWStream& pdf) {
    SkFILEWStream stream(path.c_str());
    return stream.write(pdf.getStream(), pdf.getOffset());
}

enum Backend {
  kRaster_Backend,
  kGPU_Backend,
  kPDF_Backend,
};

static const struct {
	SkBitmap::Config	fConfig;
    Backend             fBackend;
	const char*			fName;
} gRec[] = {
    { SkBitmap::kARGB_8888_Config, kRaster_Backend, "8888" },
    { SkBitmap::kARGB_4444_Config, kRaster_Backend, "4444" },
    { SkBitmap::kRGB_565_Config,   kRaster_Backend, "565" },
    { SkBitmap::kARGB_8888_Config, kGPU_Backend,    "gpu" },
#ifdef SK_SUPPORT_PDF
    { SkBitmap::kARGB_8888_Config, kPDF_Backend,    "pdf" },
#endif
};

int main (int argc, char * const argv[]) {
    SkAutoGraphics ag;

    const char* writePath = NULL;   // if non-null, where we write the originals
    const char* readPath = NULL;    // if non-null, were we read from to compare
	const char* diffPath = NULL;	// if non-null, where we write our diffs (from compare)

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
        } else if (strcmp(*argv, "-d") == 0) {
			argv++;
            if (argv < stop && **argv) {
                diffPath = *argv;
            }
		}
	}

    // setup a GL context for drawing offscreen
    GrContext* context = NULL;
    SkEGLContext eglContext;
    if (eglContext.init(1024, 1024)) {
        context = GrContext::CreateGLShaderContext();
    }

    Iter iter;
    GM* gm;

    if (readPath) {
        fprintf(stderr, "reading from %s\n", readPath);
    } else if (writePath) {
        fprintf(stderr, "writing to %s\n", writePath);
    }

    while ((gm = iter.next()) != NULL) {
		SkISize size = gm->getISize();
        SkDebugf("drawing... %s [%d %d]\n", gm->shortName(),
                 size.width(), size.height());

		SkBitmap bitmap;
        SkDynamicMemoryWStream pdf;
		for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); i++) {
            if (gRec[i].fBackend == kRaster_Backend ||
                    gRec[i].fBackend == kGPU_Backend) {
                bitmap.setConfig(gRec[i].fConfig, size.width(), size.height());
                bitmap.allocPixels();
                bitmap.eraseColor(0);
                SkCanvas canvas(bitmap);

                if (gRec[i].fBackend == kRaster_Backend) {
                    gm->draw(&canvas);
                } else {  // GPU
                    if (NULL == context) {
                        continue;
                    }
                    SkGpuCanvas gc(context,
                                   SkGpuDevice::Current3DApiRenderTarget());
                    gc.setDevice(gc.createDevice(bitmap.config(),
                                                 bitmap.width(),
                                                 bitmap.height(),
                                                 bitmap.isOpaque(),
                                                 false))->unref();
                    gm->draw(&gc);
                    gc.readPixels(&bitmap); // overwrite our previous allocation
                }
            }
            // TODO: Figure out a way to compare PDFs.
            if (gRec[i].fBackend == kPDF_Backend && writePath) {
#ifdef SK_SUPPORT_PDF
                SkISize size = gm->getISize();
                SkMatrix identity;
                identity.reset();
                SkPDFDevice* dev = new SkPDFDevice(size.width(), size.height(),
                                                   identity);
                SkAutoUnref aur(dev);

                SkCanvas c(dev);
                gm->draw(&c);

                SkPDFDocument doc;
                doc.appendPage(dev);
                doc.emitPDF(&pdf);
#endif
            }
            SkString name = make_name(gm->shortName(), gRec[i].fName);

            if (writePath) {
                SkString path;
                bool success;
                if (gRec[i].fBackend != kPDF_Backend) {
                    path = make_filename(writePath, name, "png");
                    success = write_bitmap(path, bitmap);
                } else {
                    path = make_filename(writePath, name, "pdf");
                    success = write_pdf(path, pdf);
                }
                if (!success)
                    fprintf(stderr, "FAILED to write %s\n", path.c_str());
            // TODO: Figure out a way to compare PDFs.
            } else if (readPath && gRec[i].fBackend != kPDF_Backend) {
                SkString path = make_filename(readPath, name, "png");
                SkBitmap orig;
                bool success = SkImageDecoder::DecodeFile(path.c_str(), &orig,
                                    SkBitmap::kARGB_8888_Config,
                                    SkImageDecoder::kDecodePixels_Mode, NULL);
                if (success) {
					SkBitmap diffBitmap;
                    success = compare(bitmap, orig, name, diffPath ? &diffBitmap : NULL);
					if (!success && diffPath) {
						SkString diffName = make_filename(diffPath, name, ".diff.png");
						fprintf(stderr, "Writing %s\n", diffName.c_str());
						write_bitmap(diffName, diffBitmap);
					}
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


