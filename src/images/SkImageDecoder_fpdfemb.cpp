/*
 * Copyright 2007, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); 
 * you may not use this file except in compliance with the License. 
 * You may obtain a copy of the License at 
 *
 *     http://www.apache.org/licenses/LICENSE-2.0 
 *
 * Unless required by applicable law or agreed to in writing, software 
 * distributed under the License is distributed on an "AS IS" BASIS, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
 * See the License for the specific language governing permissions and 
 * limitations under the License.
 */
 
#include "SkImageDecoder.h"
#include "SkScaledBitmapSampler.h"
#include "SkStream.h"
#include "SkColorPriv.h"
#include "SkTDArray.h"

#include "fpdfemb.h"

class SkFPDFEMBImageDecoder : public SkImageDecoder {
public:
    SkFPDFEMBImageDecoder() {}
    
    virtual Format getFormat() const {
        return kBMP_Format;
    }

protected:
    virtual bool onDecode(SkStream* stream, SkBitmap* bm,
                          SkBitmap::Config pref, Mode mode);

private:
    bool render(FPDFEMB_PAGE page, const FPDFEMB_RECT& bounds, SkBitmap* bm,
                SkBitmap::Config prefConfig, SkImageDecoder::Mode mode);
};

SkImageDecoder* SkImageDecoder_FPDFEMB_Factory(SkStream*);
SkImageDecoder* SkImageDecoder_FPDFEMB_Factory(SkStream* stream) {
    static const char kPDFSig[] = { '%', 'P', 'D', 'F' };
    
    size_t len = stream->getLength();
    char buffer[sizeof(kPDFSig)];
    
    SkDebugf("---- SkImageDecoder_FPDFEMB_Factory len=%d\n", len);
    
    if (len != 12683) { return NULL; }

    if (len > sizeof(kPDFSig) &&
            stream->read(buffer, sizeof(kPDFSig)) == sizeof(kPDFSig) &&
            !memcmp(buffer, kPDFSig, sizeof(kPDFSig))) {
        return SkNEW(SkFPDFEMBImageDecoder);
    }
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

extern "C" {
    static void* pdf_alloc(FPDFEMB_MEMMGR* pMgr, unsigned int size) {
        void* addr = sk_malloc_throw(size);
 //       SkDebugf("---- pdf_alloc %d %p\n", size, addr);
        return addr;
    }

    static void* pdf_alloc_nl(FPDFEMB_MEMMGR* pMgr, unsigned int size) {
        void* addr = sk_malloc_flags(size, 0);
 //       SkDebugf("---- pdf_alloc_nl %d %p\n", size, addr);
        return addr;
    }

    static void* pdf_realloc(FPDFEMB_MEMMGR*, void* addr, unsigned int size) {
        void* newaddr = sk_realloc_throw(addr, size);
 //       SkDebugf("---- pdf_realloc %p %d %p\n", addr, size, newaddr);
        return newaddr;
    }

    static void pdf_free(FPDFEMB_MEMMGR* pMgr, void* pointer) {
 //       SkDebugf("---- pdf_free %p\n", pointer);
        sk_free(pointer);
    }

    void FX_OUTPUT_LOG_FUNC(const char* format, ...) {
        SkDebugf("---- LOG_FUNC %s\n", format);
    }
    
    static unsigned int file_getsize(FPDFEMB_FILE_ACCESS* file) {
        SkStream* stream = (SkStream*)file->user;
        return stream->getLength();
    }
    
    static FPDFEMB_RESULT file_readblock(FPDFEMB_FILE_ACCESS* file, void* dst,
                                    unsigned int offset, unsigned int size) {
        SkStream* stream = (SkStream*)file->user;
//        SkDebugf("---- readblock %p %p %d %d\n", stream, dst, offset, size);
        if (!stream->rewind()) {
            SkDebugf("---- rewind failed\n");
            return FPDFERR_ERROR;
        }
        if (stream->skip(offset) != offset) {
            SkDebugf("---- skip failed\n");
            return FPDFERR_ERROR;
        }
        if (stream->read(dst, size) != size) {
            SkDebugf("---- read failed\n");
            return FPDFERR_ERROR;
        }
        return FPDFERR_SUCCESS;
    }

    static void pdf_oom_handler(void* memory, int size) {
        SkDebugf("======== pdf OOM %p %d\n", memory, size);
    }
}

static inline int PDF2Pixels(int x) { return x / 100; }
static inline SkScalar PDF2Scalar(int x) {
    return SkScalarMulDiv(SK_Scalar1, x, 100);
}

bool SkFPDFEMBImageDecoder::render(FPDFEMB_PAGE page, const FPDFEMB_RECT& bounds, SkBitmap* bm,
                   SkBitmap::Config prefConfig, SkImageDecoder::Mode mode) {
    int width = PDF2Pixels(bounds.right - bounds.left);
    int height = PDF2Pixels(bounds.top - bounds.bottom);

    SkDebugf("----- bitmap size [%d %d], mode=%d\n", width, height, mode);
    bm->setConfig(SkBitmap::kARGB_8888_Config, width, height);
    if (SkImageDecoder::kDecodeBounds_Mode == mode) {
        return true;
    }
    
    // USE THE CODEC TO ALLOCATE THE PIXELS!!!!
    if (!this->allocPixelRef(bm, NULL)) {
        SkDebugf("----- failed to alloc pixels\n");
        return false;
    }

    bm->eraseColor(0);
    
    FPDFEMB_RESULT result;
    FPDFEMB_BITMAP dib;

    result = FPDFEMB_CreateDIB(width, height, FPDFDIB_BGRA, bm->getPixels(),
                               bm->rowBytes(), &dib);
    SkDebugf("---- createdib %d\n", result);

    result = FPDFEMB_StartRender(dib, page, 0, 0, width, height, 0, 0, NULL, NULL);
    SkDebugf("---- render %d\n", result);
    
    result = FPDFEMB_DestroyDIB(dib);
    SkDebugf("---- destroydib %d\n", result);
    
    SkPMColor* dst = bm->getAddr32(0, 0);
    const uint8_t* src = (uint8_t*)dst;
    int n = bm->getSize() >> 2;
    for (int i = 0; i < n; i++) {
        int b = *src++;
        int g = *src++;
        int r = *src++;
        int a = *src++;
        *dst++ = SkPackARGB32(a, r, g, b);
    }

    return true;
}

#define USE_FIXED_MEM   (4 * 1024 * 1024)

bool SkFPDFEMBImageDecoder::onDecode(SkStream* stream, SkBitmap* bm,
                                 SkBitmap::Config prefConfig, Mode mode) {

    FPDFEMB_RESULT result;
#ifdef USE_FIXED_MEM
    SkAutoMalloc storage(USE_FIXED_MEM);
    result = FPDFEMB_InitFixedMemory(storage.get(), USE_FIXED_MEM,
                                     pdf_oom_handler);
#else
    FPDFEMB_MEMMGR  memmgr;
    memmgr.Alloc = pdf_alloc;
    memmgr.AllocNL = pdf_alloc_nl;
    memmgr.Realloc = pdf_realloc;
    memmgr.Free = pdf_free;

    result = FPDFEMB_Init(&memmgr);
#endif
    SkDebugf("----- SkImageDecoder_FPDFEMB_Factory init %d, streamLen = %d\n", result, stream->getLength());

    FPDFEMB_FILE_ACCESS file;
    file.GetSize = file_getsize;
    file.ReadBlock = file_readblock;
    file.user = stream;

    FPDFEMB_DOCUMENT document;
    result = FPDFEMB_StartLoadDocument(&file, NULL, &document, NULL);
    SkDebugf("----- SkImageDecoder_FPDFEMB_Factory open %d %p\n", result, document);

    int pageCount = FPDFEMB_GetPageCount(document);
    SkDebugf("----- SkImageDecoder_FPDFEMB_Factory pageCount %d\n", pageCount);

    if (pageCount > 0) {
        FPDFEMB_PAGE page;
        result = FPDFEMB_LoadPage(document, 0, &page);
        SkDebugf("----- SkImageDecoder_FPDFEMB_Factory load page %d\n", result);

        int width, height;
        result = FPDFEMB_GetPageSize(page, &width, &height);
        SkDebugf("----- SkImageDecoder_FPDFEMB_Factory page size %d [%d %d]\n", result, width, height);

        FPDFEMB_RECT rect;
        result = FPDFEMB_GetPageBBox(page, &rect);
        SkDebugf("----- SkImageDecoder_FPDFEMB_Factory page rect %d [%d %d %d %d]\n", result,
                 rect.left, rect.top, rect.right, rect.bottom);

        SkDebugf("----- SkImageDecoder_FPDFEMB_Factory begin page parse...\n");
        result = FPDFEMB_StartParse(page, false, NULL);
        SkDebugf("----- SkImageDecoder_FPDFEMB_Factory page parse %d\n", result);

        if (0 == result) {
            this->render(page, rect, bm, prefConfig, mode);
        }

        result = FPDFEMB_ClosePage(page);
        SkDebugf("----- SkImageDecoder_FPDFEMB_Factory close page %d\n", result);
    }
    
    result = FPDFEMB_CloseDocument(document);
    SkDebugf("----- SkImageDecoder_FPDFEMB_Factory close %d\n", result);

 //   FPDFEMB_Exit();

    return true;    
}
