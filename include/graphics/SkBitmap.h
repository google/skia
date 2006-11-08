/* include/graphics/SkBitmap.h
**
** Copyright 2006, Google Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#ifndef SkBitmap_DEFINED
#define SkBitmap_DEFINED

#include "SkColor.h"
#include "SkRefCnt.h"

// Android - we need to run as an embedded product, not X11
//#ifdef SK_BUILD_FOR_UNIX
//#include <X11/Xlib.h>
//#endif


class SkColorTable;

/** \class SkBitmap

    The SkBitmap class specifies a raster bitmap. A bitmap has an integer width
    and height, and a format (config), and a pointer to the actual pixels.
    Bitmaps can be drawn into a SkCanvas, but they are also used to specify the target
    of a SkCanvas' drawing operations.
*/
class SkBitmap {
public:
    enum Config {
        kNo_Config,         //!< bitmap has not been configured
        kA1_Config,         //!< 1-bit per pixel, (0 is transparent, 1 is opaque)
        kA8_Config,         //!< 8-bits per pixel, with only alpha specified (0 is transparent, 0xFF is opaque)
        kIndex8_Config,     //!< 8-bits per pixel, using SkColorTable to specify the colors
        kRGB_565_Config,    //!< 16-bits per pixel, (see SkColorPriv.h for packing)
        kARGB_8888_Config,  //!< 32-bits per pixel, (see SkColorPriv.h for packing)

        kConfigCount
    };

    /** Default construct creates a bitmap with zero width and height, and no pixels.
        Its config is set to kNo_Config.
    */
    SkBitmap();
    /** Constructor initializes the new bitmap by copying the src bitmap. All fields are copied,
        but ownership of the pixels remains with the src bitmap.
    */
    //  This method is not exported to java.
    SkBitmap(const SkBitmap& src);
    /** Destructor that, if getOwnsPixels() returns true, will delete the pixel's memory.
    */
    ~SkBitmap();

    /** Copies the src bitmap into this bitmap. Ownership of the src bitmap's pixels remains
        with the src bitmap.
    */
    SkBitmap& operator=(const SkBitmap& src);
    /** Swap the fields of the two bitmaps. This routine is guaranteed to never fail or throw.
    */
    //  This method is not exported to java.
    void    swap(SkBitmap& other);

    /** Return the config for the bitmap.
    */
    Config  getConfig() const { return (Config)fConfig; }
    /** Return the bitmap's width, in pixels.
    */
    int width() const { return fWidth; }
    /** Return the bitmap's height, in pixels.
    */
    int height() const { return fHeight; }
    /** Return the number of bytes between subsequent rows of the bitmap.
    */
    int rowBytes() const { return fRowBytes; }
    /** Return the address of the pixels for this SkBitmap. This can be set either with
        setPixels(), where the caller owns the buffer, or with allocPixels() or resizeAlloc(),
        which marks the pixel memory to be owned by the SkBitmap (e.g. will be freed automatically
        when the bitmap is destroyed).
    */
    void*   getPixels() const { return fPixels; }
    /** Return the byte size of the pixels, based on the height and rowBytes
    */
    size_t  getSize() const { return fHeight * fRowBytes; }
    
    /** Returns true if the bitmap is opaque (has no translucent/transparent pixels).
    */
    bool    isOpaque() const;
    /** Specify if this bitmap's pixels are all opaque or not. Is only meaningful for configs
        that support per-pixel alpha (RGB32, A1, A8).
    */
    void    setIsOpaque(bool);

    /** Reset the bitmap to its initial state (see default constructor). If getOwnsPixels() returned
        true, then the memory for the pixels is freed.
    */
    void    reset();
    /** Set the bitmap's config and dimensions. If rowBytes is 0, then an appropriate value
        is computed based on the bitmap's config and width. If getOwnsPixels() returned true,
        then the pixel's memory is freed.
    */
    void    setConfig(Config, U16CPU width, U16CPU height, U16CPU rowBytes = 0);
    /** Use this to assign a new pixel address for an existing bitmap. If getOwnsPixels() returned
        true, then the previous pixel's memory is freed. The new address is "owned" by the called,
        and getOwnsPixels() will now return false. This method is not exported to java.
    */
    void    setPixels(void* p);
    /** If this is called, then the bitmap will dynamically allocate memory for its pixels
        based on rowBytes and height. The SkBitmap will remember that it allocated
        this, and will automatically free it as needed, thus getOwnsPixels() will now return true.
    */
    void    allocPixels();
    /** Realloc the memory for the pixels based on the specified width and height. This
        keeps the old value for config, and computes a rowBytes based on the config and the width.
        This is similar, but more efficient than calling setConfig() followed by allocPixels().
    */
// not implemented
//  void    resizeAlloc(U16CPU width, U16CPU height);

    /** Returns true if the current pixels have been allocated via allocPixels()
        or resizeAlloc(). This method is not exported to java.
    */
    bool    getOwnsPixels() const;
    /** Call this to explicitly change the ownership rule for the pixels. This may be called
        after one bitmap is copied into another, to specify which bitmap should handle freeing
        the memory. This method is not exported to java.
    */
    void    setOwnsPixels(bool ownsPixels);

    /** Get the bitmap's colortable object.
    
    Return the bitmap's colortable (if any). Does not affect the colortable's
        reference count.
    */
    SkColorTable* getColorTable() const { return fColorTable; }
    /** Assign ctable to be the colortable for the bitmap, replacing any existing reference.
        The reference count of ctable (if it is not nil) is incremented, and any existing
        reference has its reference count decremented. NOTE: colortable's may be assigned
        to any bitmap, but are only interpreted for kIndex8_Config bitmaps, where they
        are required.
        @return the ctable argument
    */
    SkColorTable* setColorTable(SkColorTable* ctable);

    /** Initialize the bitmap's pixels with the specified color+alpha, automatically converting into the correct format
        for the bitmap's config. If the config is kRGB_565_Config, then the alpha value is ignored.
        If the config is kA8_Config, then the r,g,b parameters are ignored.
    */
    void eraseARGB(U8CPU a, U8CPU r, U8CPU g, U8CPU b);
    /** Initialize the bitmap's pixels with the specified color+alpha, automatically converting into the correct format
        for the bitmap's config. If the config is kRGB_565_Config, then the alpha value is presumed
        to be 0xFF. If the config is kA8_Config, then the r,g,b parameters are ignored and the
        pixels are all set to 0xFF.
    */
    void eraseRGB(U8CPU r, U8CPU g, U8CPU b)
    {
        this->eraseARGB(0xFF, r, g, b);
    }
    /** Initialize the bitmap's pixels with the specified color, automatically converting into the correct format
        for the bitmap's config. If the config is kRGB_565_Config, then the color's alpha value is presumed
        to be 0xFF. If the config is kA8_Config, then only the color's alpha value is used.
    */
    void eraseColor(SkColor c)
    {
        this->eraseARGB(SkColorGetA(c), SkColorGetR(c), SkColorGetG(c), SkColorGetB(c));
    }

    /** Return a bitmap that is a quarter the size of this one
    */
    bool quarterSizeFiltered(SkBitmap* dst) const;
    
    /** Returns the address of the pixel specified by x,y.
        Asserts that x,y are in range, and that the bitmap's config is either kARGB_8888_Config.
    */
    //  This method is not exported to java.
    inline uint32_t* getAddr32(int x, int y) const;
    /** Returns the address of the pixel specified by x,y.
        Asserts that x,y are in range, and that the bitmap's config is kRGB_565_Config.
    */
    //  This method is not exported to java.
    inline uint16_t* getAddr16(int x, int y) const;
    /** Returns the address of the pixel specified by x,y.
        Asserts that x,y are in range, and that the bitmap's config is either kA8_Config or kIndex8_Config.
    */
    //  This method is not exported to java.
    inline uint8_t* getAddr8(int x, int y) const;
    /** Returns the color corresponding to the pixel specified by x,y.
        Asserts that x,y are in range, and that the bitmap's config is kIndex8_Config.
    */
    //  This method is not exported to java.
    inline SkPMColor getIndex8Color(int x, int y) const;
    /** Returns the address of the byte containing the pixel specified by x,y.
        Asserts that x,y are in range, and that the bitmap's config is kA1_Config.
    */
    //  This method is not exported to java.
    inline uint8_t* getAddr1(int x, int y) const;

    //  OS-specific helpers
#ifndef SK_USE_WXWIDGETS
#ifdef SK_BUILD_FOR_WIN
    /** On Windows and PocketPC builds, this will draw the SkBitmap onto the
        specified HDC
    */
    void    drawToHDC(HDC, int left, int top) const;
#elif defined(SK_BUILD_FOR_MAC)
    /** On Mac OS X and Carbon builds, this will draw the SkBitmap onto the
        specified WindowRef
    */
    void    drawToPort(WindowRef) const;
#endif
#endif

    void        buildMipMap(bool forceRebuild);
    unsigned    countMipLevels() const;

private:
    SkColorTable*   fColorTable;    // only meaningful for kIndex8

#ifdef SK_SUPPORT_MIPMAP
    struct MipLevel {
        void*       fPixels;
        uint16_t    fWidth, fHeight, fRowBytes;
        uint8_t     fConfig, fShift;
    };
    enum {
        kMaxMipLevels = 5
    };
    struct MipMap {
        MipLevel    fLevel[kMaxMipLevels];
    };
    MipMap* fMipMap;
#endif

    enum Flags {
        kWeOwnThePixels_Flag = 0x01,
        kWeOwnTheMipMap_Flag = 0x02,
        kImageIsOpaque_Flag  = 0x04
    };

    void*       fPixels;
    uint16_t    fWidth, fHeight, fRowBytes;
    uint8_t     fConfig;
    uint8_t     fFlags;

#ifdef SK_SUPPORT_MIPMAP
    const MipLevel* getMipLevel(unsigned level) const;
#endif
    void freePixels();

    friend class SkBitmapShader;
};

/** \class SkColorTable

    SkColorTable holds an array SkPMColors (premultiplied 32-bit colors) used by
    8-bit bitmaps, where the bitmap bytes are interpreted as indices into the colortable.
*/
class SkColorTable : public SkRefCnt {
public:
    /** Constructs an empty color table (zero colors).
    */
            SkColorTable();
    virtual ~SkColorTable();

    enum Flags {
        kColorsAreOpaque_Flag   = 0x01  //!< if set, all of the colors in the table are opaque (alpha==0xFF)
    };
    /** Returns the flag bits for the color table. These can be changed with setFlags().
    */
    unsigned getFlags() const { return fFlags; }
    /** Set the flags for the color table. See the Flags enum for possible values.
    */
    void    setFlags(unsigned flags);

    /** Returns the number of colors in the table.
    */
    int count() const { return fCount; }

    /** Returns the specified color from the table. In the debug build, this asserts that
        the index is in range (0 <= index < count).
    */
    SkPMColor operator[](int index) const
    {
        SkASSERT(fColors != nil && (unsigned)index < fCount);
        return fColors[index];
    }

    /** Specify the number of colors in the color table. This does not initialize the colors
        to any value, just allocates memory for them. To initialize the values, either call
        setColors(array, count), or follow setCount(count) with a call to
        lockColors()/{set the values}/unlockColors(true).
    */
    void    setColors(int count) { this->setColors(nil, count); }
    void    setColors(const SkPMColor[], int count);

    /** Return the array of colors for reading and/or writing. This must be
        balanced by a call to unlockColors(changed?), telling the colortable if
        the colors were changed during the lock.
    */
    SkPMColor* lockColors()
    {
        SkDEBUGCODE(fColorLockCount += 1;)
        return fColors;
    }
    /** Balancing call to lockColors(). If the colors have been changed, pass true.
    */
    void unlockColors(bool changed)
    {
        SkASSERT(fColorLockCount != 0);
        SkDEBUGCODE(fColorLockCount -= 1;)
    }

    /** Similar to lockColors(), lock16BitCache() returns the array of
        RGB16 colors that mirror the 32bit colors. However, this function
        will return nil if kColorsAreOpaque_Flag is not set.
        Also, unlike lockColors(), the returned array here cannot be modified.
    */
    const uint16_t* lock16BitCache();
    /** Balancing call to lock16BitCache().
    */
    void        unlock16BitCache()
    {
        SkASSERT(f16BitCacheLockCount > 0);
        SkDEBUGCODE(f16BitCacheLockCount -= 1);
    }

private:
    SkPMColor*  fColors;
    uint16_t*   f16BitCache;
    uint16_t    fCount;
    uint8_t     fFlags;
    SkDEBUGCODE(int fColorLockCount;)
    SkDEBUGCODE(int f16BitCacheLockCount;)

    void inval16BitCache();
};

//////////////////////////////////////////////////////////////////////////////////

inline uint32_t* SkBitmap::getAddr32(int x, int y) const
{
    SkASSERT(fPixels);
    SkASSERT(fConfig == kARGB_8888_Config);
    SkASSERT((unsigned)x < fWidth && (unsigned)y < fHeight);

    return (uint32_t*)((char*)fPixels + y * fRowBytes + (x << 2));
}

inline uint16_t* SkBitmap::getAddr16(int x, int y) const
{
    SkASSERT(fPixels);
    SkASSERT(fConfig == kRGB_565_Config);
    SkASSERT((unsigned)x < fWidth && (unsigned)y < fHeight);

    return (uint16_t*)((char*)fPixels + y * fRowBytes + (x << 1));
}

inline uint8_t* SkBitmap::getAddr8(int x, int y) const
{
    SkASSERT(fPixels);
    SkASSERT(fConfig == kA8_Config || fConfig == kIndex8_Config);
    SkASSERT((unsigned)x < fWidth && (unsigned)y < fHeight);
    return (uint8_t*)fPixels + y * fRowBytes + x;
}

inline SkPMColor SkBitmap::getIndex8Color(int x, int y) const
{
    SkASSERT(fPixels);
    SkASSERT(fConfig == kIndex8_Config);
    SkASSERT((unsigned)x < fWidth && (unsigned)y < fHeight);
    SkASSERT(fColorTable);
    return (*fColorTable)[*((const uint8_t*)fPixels + y * fRowBytes + x)];
}

// returns the address of the byte that contains the x coordinate
inline uint8_t* SkBitmap::getAddr1(int x, int y) const
{
    SkASSERT(fPixels);
    SkASSERT(fConfig == kA1_Config);
    SkASSERT((unsigned)x < fWidth && (unsigned)y < fHeight);
    return (uint8_t*)fPixels + y * fRowBytes + (x >> 3);
}


#endif

