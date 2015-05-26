/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPixmap_DEFINED
#define SkPixmap_DEFINED

#include "SkImageInfo.h"

class SkColorTable;

/**
 *  Pairs SkImageInfo with actual pixels and rowbytes. This class does not try to manage the
 *  lifetime of the pixel memory (nor the colortable if provided).
 */
class SkPixmap {
public:
    SkPixmap()
        : fPixels(NULL), fCTable(NULL), fRowBytes(0), fInfo(SkImageInfo::MakeUnknown(0, 0))
    {}

    SkPixmap(const SkImageInfo& info, const void* addr, size_t rowBytes,
             SkColorTable* ctable = NULL)
        : fPixels(addr), fCTable(ctable), fRowBytes(rowBytes), fInfo(info)
    {
        if (kIndex_8_SkColorType == info.colorType()) {
            SkASSERT(ctable);
        } else {
            SkASSERT(NULL == ctable);
        }
    }

    void reset();
    void reset(const SkImageInfo& info, const void* addr, size_t rowBytes,
               SkColorTable* ctable = NULL);

    const SkImageInfo& info() const { return fInfo; }
    size_t rowBytes() const { return fRowBytes; }
    const void* addr() const { return fPixels; }
    SkColorTable* ctable() const { return fCTable; }

    int width() const { return fInfo.width(); }
    int height() const { return fInfo.height(); }
    SkColorType colorType() const { return fInfo.colorType(); }
    SkAlphaType alphaType() const { return fInfo.alphaType(); }
    bool isOpaque() const { return fInfo.isOpaque(); }

    uint64_t getSize64() const { return sk_64_mul(fInfo.height(), fRowBytes); }
    uint64_t getSafeSize64() const { return fInfo.getSafeSize64(fRowBytes); }
    size_t getSafeSize() const { return fInfo.getSafeSize(fRowBytes); }

    const uint32_t* addr32() const {
        SkASSERT(4 == SkColorTypeBytesPerPixel(fInfo.colorType()));
        return reinterpret_cast<const uint32_t*>(fPixels);
    }

    const uint16_t* addr16() const {
        SkASSERT(2 == SkColorTypeBytesPerPixel(fInfo.colorType()));
        return reinterpret_cast<const uint16_t*>(fPixels);
    }

    const uint8_t* addr8() const {
        SkASSERT(1 == SkColorTypeBytesPerPixel(fInfo.colorType()));
        return reinterpret_cast<const uint8_t*>(fPixels);
    }

    const uint32_t* addr32(int x, int y) const {
        SkASSERT((unsigned)x < (unsigned)fInfo.width());
        SkASSERT((unsigned)y < (unsigned)fInfo.height());
        return (const uint32_t*)((const char*)this->addr32() + y * fRowBytes + (x << 2));
    }
    const uint16_t* addr16(int x, int y) const {
        SkASSERT((unsigned)x < (unsigned)fInfo.width());
        SkASSERT((unsigned)y < (unsigned)fInfo.height());
        return (const uint16_t*)((const char*)this->addr16() + y * fRowBytes + (x << 1));
    }
    const uint8_t* addr8(int x, int y) const {
        SkASSERT((unsigned)x < (unsigned)fInfo.width());
        SkASSERT((unsigned)y < (unsigned)fInfo.height());
        return (const uint8_t*)((const char*)this->addr8() + y * fRowBytes + (x << 0));
    }
    const void* addr(int x, int y) const {
        return (const char*)fPixels + fInfo.computeOffset(x, y, fRowBytes);
    }

    // Writable versions

    void* writable_addr() const { return const_cast<void*>(fPixels); }
    uint32_t* writable_addr32(int x, int y) const {
        return const_cast<uint32_t*>(this->addr32(x, y));
    }
    uint16_t* writable_addr16(int x, int y) const {
        return const_cast<uint16_t*>(this->addr16(x, y));
    }
    uint8_t* writable_addr8(int x, int y) const {
        return const_cast<uint8_t*>(this->addr8(x, y));
    }

    // copy methods

    bool readPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes,
                    int srcX, int srcY) const;
    bool readPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes) const {
        return this->readPixels(dstInfo, dstPixels, dstRowBytes, 0, 0);
    }
    bool readPixels(const SkPixmap& dst, int srcX, int srcY) const {
        return this->readPixels(dst.info(), dst.writable_addr(), dst.rowBytes(), srcX, srcY);
    }
    bool readPixels(const SkPixmap& dst) const {
        return this->readPixels(dst.info(), dst.writable_addr(), dst.rowBytes(), 0, 0);
    }

private:
    const void*     fPixels;
    SkColorTable*   fCTable;
    size_t          fRowBytes;
    SkImageInfo     fInfo;
};

/////////////////////////////////////////////////////////////////////////////////////////////

class SkAutoPixmapUnlock : ::SkNoncopyable {
public:
    SkAutoPixmapUnlock() : fUnlockProc(NULL), fIsLocked(false) {}
    SkAutoPixmapUnlock(const SkPixmap& pm, void (*unlock)(void*), void* ctx)
        : fUnlockProc(unlock), fUnlockContext(ctx), fPixmap(pm), fIsLocked(true)
    {}
    ~SkAutoPixmapUnlock() { this->unlock(); }

    /**
     *  Return the currently locked pixmap. Undefined if it has been unlocked.
     */
    const SkPixmap& pixmap() const {
        SkASSERT(this->isLocked());
        return fPixmap;
    }

    bool isLocked() const { return fIsLocked; }

    /**
     *  Unlocks the pixmap. Can safely be called more than once as it will only call the underlying
     *  unlock-proc once.
     */
    void unlock() {
        if (fUnlockProc) {
            SkASSERT(fIsLocked);
            fUnlockProc(fUnlockContext);
            fUnlockProc = NULL;
            fIsLocked = false;
        }
    }

    /**
     *  If there is a currently locked pixmap, unlock it, then copy the specified pixmap
     *  and (optional) unlock proc/context.
     */
    void reset(const SkPixmap& pm, void (*unlock)(void*), void* ctx);

private:
    void        (*fUnlockProc)(void*);
    void*       fUnlockContext;
    SkPixmap    fPixmap;
    bool        fIsLocked;

    friend class SkBitmap;
};

#endif
