/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorPriv.h"
#include "SkImageDecoder.h"
#include "SkStream.h"
#include "SkStreamPriv.h"
#include "SkTypes.h"

class SkICOImageDecoder : public SkImageDecoder {
public:
    SkICOImageDecoder();

    Format getFormat() const override {
        return kICO_Format;
    }

protected:
    Result onDecode(SkStream* stream, SkBitmap* bm, Mode) override;

private:
    typedef SkImageDecoder INHERITED;
};

/////////////////////////////////////////////////////////////////////////////////////////

//read bytes starting from the begin-th index in the buffer
//read in Intel order, and return an integer

#define readByte(buffer,begin) buffer[begin]
#define read2Bytes(buffer,begin) buffer[begin]+(buffer[begin+1]<<8)
#define read4Bytes(buffer,begin) buffer[begin]+(buffer[begin+1]<<8)+(buffer[begin+2]<<16)+(buffer[begin+3]<<24)

/////////////////////////////////////////////////////////////////////////////////////////

SkICOImageDecoder::SkICOImageDecoder()
{
}

//helpers - my function pointer will call one of these, depending on the bitCount, each time through the inner loop
static void editPixelBit1(const int pixelNo, const unsigned char* buf,
            const int xorOffset, int& x, int y, const int w,
            SkBitmap* bm, int alphaByte, int m, int shift, SkPMColor* colors);
static void editPixelBit4(const int pixelNo, const unsigned char* buf,
            const int xorOffset, int& x, int y, const int w,
            SkBitmap* bm, int alphaByte, int m, int shift, SkPMColor* colors);
static void editPixelBit8(const int pixelNo, const unsigned char* buf,
            const int xorOffset, int& x, int y, const int w,
            SkBitmap* bm, int alphaByte, int m, int shift, SkPMColor* colors);
static void editPixelBit24(const int pixelNo, const unsigned char* buf,
            const int xorOffset, int& x, int y, const int w,
            SkBitmap* bm, int alphaByte, int m, int shift, SkPMColor* colors);
static void editPixelBit32(const int pixelNo, const unsigned char* buf,
            const int xorOffset, int& x, int y, const int w,
            SkBitmap* bm, int alphaByte, int m, int shift, SkPMColor* colors);


static int calculateRowBytesFor8888(int w, int bitCount)
{
    //  Default rowBytes is w << 2 for kARGB_8888
    //  In the case of a 4 bit image with an odd width, we need to add some
    //  so we can go off the end of the drawn bitmap.
    //  Add 4 to ensure that it is still a multiple of 4.
    if (4 == bitCount && (w & 0x1)) {
        return (w + 1) << 2;
    }
    //  Otherwise return 0, which will allow it to be calculated automatically.
    return 0;
}

SkImageDecoder::Result SkICOImageDecoder::onDecode(SkStream* stream, SkBitmap* bm, Mode mode)
{
    SkAutoMalloc autoMal;
    const size_t length = SkCopyStreamToStorage(&autoMal, stream);
    // Check that the buffer is large enough to read the directory header
    if (length < 6) {
        return kFailure;
    }

    unsigned char* buf = (unsigned char*)autoMal.get();

    //these should always be the same - should i use for error checking? - what about files that have some
    //incorrect values, but still decode properly?
    int reserved = read2Bytes(buf, 0);    // 0
    int type = read2Bytes(buf, 2);        // 1
    if (reserved != 0 || type != 1) {
        return kFailure;
    }

    int count = read2Bytes(buf, 4);
    // Check that there are directory entries
    if (count < 1) {
        return kFailure;
    }

    // Check that buffer is large enough to read directory entries.
    // We are guaranteed that count is at least 1.  We might as well assume
    // count is 1 because this deprecated decoder only looks at the first
    // directory entry.
    if (length < (size_t)(6 + count*16)) {
        return kFailure;
    }

    //skip ahead to the correct header
    //commented out lines are not used, but if i switch to other read method, need to know how many to skip
    //otherwise, they could be used for error checking
    int w = readByte(buf, 6);
    int h = readByte(buf, 7);
    SkASSERT(w >= 0 && h >= 0);
    int colorCount = readByte(buf, 8);
    //int reservedToo = readByte(buf, 9 + choice*16);   //0
    //int planes = read2Bytes(buf, 10 + choice*16);       //1 - but often 0
    //int fakeBitCount = read2Bytes(buf, 12 + choice*16); //should be real - usually 0
    const size_t size = read4Bytes(buf, 14);           //matters?
    const size_t offset = read4Bytes(buf, 18);
    // promote the sum to 64-bits to avoid overflow
    // Check that buffer is large enough to read image data
    if (offset > length || size > length || ((uint64_t)offset + size) > length) {
        return kFailure;
    }

    // Check to see if this is a PNG image inside the ICO
    {
        SkMemoryStream subStream(buf + offset, size, false);
        SkAutoTDelete<SkImageDecoder> otherDecoder(SkImageDecoder::Factory(&subStream));
        if (otherDecoder.get() != nullptr) {
            // Disallow nesting ICO files within one another
            // FIXME: Can ICO files contain other formats besides PNG?
            if (otherDecoder->getFormat() == SkImageDecoder::kICO_Format) {
                return kFailure;
            }
            // Set fields on the other decoder to be the same as this one.
            this->copyFieldsToOther(otherDecoder.get());
            const Result result = otherDecoder->decode(&subStream, bm, this->getDefaultPref(),
                                                       mode);
            // FIXME: Should we just return result here? Is it possible that data that looked like
            // a subimage was not, but was actually a valid ICO?
            if (result != kFailure) {
                return result;
            }
        }
    }

    //int infoSize = read4Bytes(buf, offset);             //40
    //int width = read4Bytes(buf, offset+4);              //should == w
    //int height = read4Bytes(buf, offset+8);             //should == 2*h
    //int planesToo = read2Bytes(buf, offset+12);         //should == 1 (does it?)
    
    // For ico images, only a byte is used to store each dimension
    // 0 is used to represent 256
    if (w == 0) {
        w = 256;
    }
    if (h == 0) {
        h = 256;
    }

    // Check that buffer is large enough to read the bit depth
    if (length < offset + 16) {
        return kFailure;
    }
    int bitCount = read2Bytes(buf, offset+14);

    void (*placePixel)(const int pixelNo, const unsigned char* buf,
        const int xorOffset, int& x, int y, const int w,
        SkBitmap* bm, int alphaByte, int m, int shift, SkPMColor* colors) = nullptr;
    switch (bitCount)
    {
        case 1:
            placePixel = &editPixelBit1;
            colorCount = 2;
            break;
        case 4:
            placePixel = &editPixelBit4;
            colorCount = 16;
            break;
        case 8:
            placePixel = &editPixelBit8;
            colorCount = 256;
            break;
        case 24:
            placePixel = &editPixelBit24;
            colorCount = 0;
            break;
        case 32:
            placePixel = &editPixelBit32;
            colorCount = 0;
            break;
        default:
            SkDEBUGF(("Decoding %ibpp is unimplemented\n", bitCount));
            return kFailure;
    }

    //these should all be zero, but perhaps are not - need to check
    //int compression = read4Bytes(buf, offset+16);       //0
    //int imageSize = read4Bytes(buf, offset+20);         //0 - sometimes has a value
    //int xPixels = read4Bytes(buf, offset+24);           //0
    //int yPixels = read4Bytes(buf, offset+28);           //0
    //int colorsUsed = read4Bytes(buf, offset+32)         //0 - might have an actual value though
    //int colorsImportant = read4Bytes(buf, offset+36);   //0

    int begin = SkToInt(offset + 40);
    // Check that the buffer is large enough to read the color table
    // For bmp-in-icos, there should be 4 bytes per color
    if (length < (size_t) (begin + 4*colorCount)) {
        return kFailure;
    }

    //this array represents the colortable
    //if i allow other types of bitmaps, it may actually be used as a part of the bitmap
    SkPMColor* colors = nullptr;
    int blue, green, red;
    if (colorCount)
    {
        colors = new SkPMColor[colorCount];
        for (int j = 0; j < colorCount; j++)
        {
            //should this be a function - maybe a #define?
            blue = readByte(buf, begin + 4*j);
            green = readByte(buf, begin + 4*j + 1);
            red = readByte(buf, begin + 4*j + 2);
            colors[j] = SkPackARGB32(0xFF, red & 0xFF, green & 0xFF, blue & 0xFF);
        }
    }
    int bitWidth = w*bitCount;
    int test = bitWidth & 0x1F;
    int mask = -(((test >> 4) | (test >> 3) | (test >> 2) | (test >> 1) | test) & 0x1);    //either 0xFFFFFFFF or 0
    int lineBitWidth = (bitWidth & 0xFFFFFFE0) + (0x20 & mask);
    int lineWidth = lineBitWidth/bitCount;

    int xorOffset = begin + colorCount*4;   //beginning of the color bitmap
                                            //other read method means we will just be here already
    int andOffset = xorOffset + ((lineWidth*h*bitCount) >> 3);

    /*int */test = w & 0x1F;   //the low 5 bits - we are rounding up to the next 32 (2^5)
    /*int */mask = -(((test >> 4) | (test >> 3) | (test >> 2) | (test >> 1) | test) & 0x1);    //either 0xFFFFFFFF or 0
    int andLineWidth = (w & 0xFFFFFFE0) + (0x20 & mask);
    //if we allow different Configs, everything is the same til here
    //change the config, and use different address getter, and place index vs color, and add the color table
    //FIXME: what is the tradeoff in size?
    //if the andbitmap (mask) is all zeroes, then we can easily do an index bitmap
    //however, with small images with large colortables, maybe it's better to still do argb_8888

    bm->setInfo(SkImageInfo::MakeN32Premul(w, h), calculateRowBytesFor8888(w, bitCount));

    if (SkImageDecoder::kDecodeBounds_Mode == mode) {
        delete[] colors;
        return kSuccess;
    }

    if (!this->allocPixelRef(bm, nullptr))
    {
        delete[] colors;
        return kFailure;
    }

    // The AND mask is a 1-bit alpha mask for each pixel that comes after the
    // XOR mask in the bmp.  If we check that the largest AND offset is safe,
    // it should mean all other buffer accesses will be at smaller indices and
    // will therefore be safe.
    size_t maxAndOffset = andOffset + ((andLineWidth*(h-1)+(w-1)) >> 3);
    if (length <= maxAndOffset) {
        return kFailure;
    }

    // Here we assert that all reads from the buffer using the XOR offset are
    // less than the AND offset.  This should be guaranteed based on the above
    // calculations.
#ifdef SK_DEBUG
    int maxPixelNum = lineWidth*(h-1)+w-1;
    int maxByte;
    switch (bitCount) {
        case 1:
            maxByte = maxPixelNum >> 3;
            break;
        case 4:
            maxByte = maxPixelNum >> 1;
            break;
        case 8:
            maxByte = maxPixelNum;
            break;
        case 24:
            maxByte = maxPixelNum * 3 + 2;
            break;
        case 32:
            maxByte = maxPixelNum * 4 + 3;
            break;
        default:
            SkASSERT(false);
            return kFailure;
    }
    int maxXOROffset = xorOffset + maxByte;
    SkASSERT(maxXOROffset < andOffset);
#endif

    SkAutoLockPixels alp(*bm);

    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            //U32* address = bm->getAddr32(x, y);

            //check the alpha bit first, but pass it along to the function to figure out how to deal with it
            int andPixelNo = andLineWidth*(h-y-1)+x;
            //only need to get a new alphaByte when x %8 == 0
            //but that introduces an if and a mod - probably much slower
            //that's ok, it's just a read of an array, not a stream
            int alphaByte = readByte(buf, andOffset + (andPixelNo >> 3));
            int shift = 7 - (andPixelNo & 0x7);
            int m = 1 << shift;

            int pixelNo = lineWidth*(h-y-1)+x;
            placePixel(pixelNo, buf, xorOffset, x, y, w, bm, alphaByte, m, shift, colors);

        }
    }

    delete [] colors;
    //ensure we haven't read off the end?
    //of course this doesn't help us if the andOffset was a lie...
    //return andOffset + (andLineWidth >> 3) <= length;
    return kSuccess;
}   //onDecode

//function to place the pixel, determined by the bitCount
static void editPixelBit1(const int pixelNo, const unsigned char* buf,
            const int xorOffset, int& x, int y, const int w,
            SkBitmap* bm, int alphaByte, int m, int shift, SkPMColor* colors)
{
    // note that this should be the same as/similar to the AND bitmap
    SkPMColor* address = bm->getAddr32(x,y);
    int byte = readByte(buf, xorOffset + (pixelNo >> 3));
    int colorBit;
    int alphaBit;
    // Read all of the bits in this byte.
    int i = x + 8;
    // Pin to the width so we do not write outside the bounds of
    // our color table.
    i = i > w ? w : i;
    // While loop to check all 8 bits individually.
    while (x < i)
    {

        colorBit = (byte & m) >> shift;
        alphaBit = (alphaByte & m) >> shift;
        *address = (alphaBit-1)&(colors[colorBit]);
        x++;
        // setup for the next pixel
        address = address + 1;
        m = m >> 1;
        shift -= 1;
    }
    x--;
}
static void editPixelBit4(const int pixelNo, const unsigned char* buf,
            const int xorOffset, int& x, int y, const int w,
            SkBitmap* bm, int alphaByte, int m, int shift, SkPMColor* colors)
{
    SkPMColor* address = bm->getAddr32(x, y);
    int byte = readByte(buf, xorOffset + (pixelNo >> 1));
    int pixel = (byte >> 4) & 0xF;
    int alphaBit = (alphaByte & m) >> shift;
    *address = (alphaBit-1)&(colors[pixel]);
    x++;
    //if w is odd, x may be the same as w, which means we are writing to an unused portion of the bitmap
    //but that's okay, since i've added an extra rowByte for just this purpose
    address = address + 1;
    pixel = byte & 0xF;
    m = m >> 1;
    alphaBit = (alphaByte & m) >> (shift-1);
    //speed up trick here
    *address = (alphaBit-1)&(colors[pixel]);
}

static void editPixelBit8(const int pixelNo, const unsigned char* buf,
            const int xorOffset, int& x, int y, const int w,
            SkBitmap* bm, int alphaByte, int m, int shift, SkPMColor* colors)
{
    SkPMColor* address = bm->getAddr32(x, y);
    int pixel = readByte(buf, xorOffset + pixelNo);
    int alphaBit = (alphaByte & m) >> shift;
    *address = (alphaBit-1)&(colors[pixel]);
}

static void editPixelBit24(const int pixelNo, const unsigned char* buf,
            const int xorOffset, int& x, int y, const int w,
            SkBitmap* bm, int alphaByte, int m, int shift, SkPMColor* colors)
{
    SkPMColor* address = bm->getAddr32(x, y);
    int blue = readByte(buf, xorOffset + 3*pixelNo);
    int green = readByte(buf, xorOffset + 3*pixelNo + 1);
    int red = readByte(buf, xorOffset + 3*pixelNo + 2);
    int alphaBit = (alphaByte & m) >> shift;
    //alphaBit == 1 => alpha = 0
    int alpha = (alphaBit-1) & 0xFF;
    *address = SkPreMultiplyARGB(alpha, red, green, blue);
}

static void editPixelBit32(const int pixelNo, const unsigned char* buf,
            const int xorOffset, int& x, int y, const int w,
            SkBitmap* bm, int alphaByte, int m, int shift, SkPMColor* colors)
{
    SkPMColor* address = bm->getAddr32(x, y);
    int blue = readByte(buf, xorOffset + 4*pixelNo);
    int green = readByte(buf, xorOffset + 4*pixelNo + 1);
    int red = readByte(buf, xorOffset + 4*pixelNo + 2);
    int alphaBit = (alphaByte & m) >> shift;
#if 1 // don't trust the alphaBit for 32bit images <mrr>
    alphaBit = 0;
#endif
    int alpha = readByte(buf, xorOffset + 4*pixelNo + 3) & ((alphaBit-1)&0xFF);
    *address = SkPreMultiplyARGB(alpha, red, green, blue);
}

///////////////////////////////////////////////////////////////////////////////
DEFINE_DECODER_CREATOR(ICOImageDecoder);
/////////////////////////////////////////////////////////////////////////////////////////

static bool is_ico(SkStreamRewindable* stream) {
    // Check to see if the first four bytes are 0,0,1,0
    // FIXME: Is that required and sufficient?
    char buf[4];
    if (stream->read((void*)buf, 4) != 4) {
        return false;
    }
    int reserved = read2Bytes(buf, 0);
    int type = read2Bytes(buf, 2);
    return 0 == reserved && 1 == type;
}

static SkImageDecoder* sk_libico_dfactory(SkStreamRewindable* stream) {
    if (is_ico(stream)) {
        return new SkICOImageDecoder;
    }
    return nullptr;
}

static SkImageDecoder_DecodeReg gReg(sk_libico_dfactory);

static SkImageDecoder::Format get_format_ico(SkStreamRewindable* stream) {
    if (is_ico(stream)) {
        return SkImageDecoder::kICO_Format;
    }
    return SkImageDecoder::kUnknown_Format;
}

static SkImageDecoder_FormatReg gFormatReg(get_format_ico);
