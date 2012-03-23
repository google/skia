
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkImageDecoder.h"
#include "SkStream.h"
#include "SkColorPriv.h"
#include "SkTypes.h"

class SkICOImageDecoder : public SkImageDecoder {
public:
    SkICOImageDecoder();
    
    virtual Format getFormat() const {
        return kICO_Format;
    }

protected:
    virtual bool onDecode(SkStream* stream, SkBitmap* bm, Mode);
};

SkImageDecoder* SkCreateICOImageDecoder() {
    return new SkICOImageDecoder;
}

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

bool SkICOImageDecoder::onDecode(SkStream* stream, SkBitmap* bm, Mode mode)
{
    size_t length = stream->read(NULL, 0);
    SkAutoMalloc autoMal(length);
    unsigned char* buf = (unsigned char*)autoMal.get();
    if (stream->read((void*)buf, length) != length) {
        return false;
    }
    
    //these should always be the same - should i use for error checking? - what about files that have some
    //incorrect values, but still decode properly?
    int reserved = read2Bytes(buf, 0);    // 0
    int type = read2Bytes(buf, 2);        // 1
    if (reserved != 0 || type != 1)
        return false;
    int count = read2Bytes(buf, 4);
    
    //need to at least have enough space to hold the initial table of info
    if (length < (size_t)(6 + count*16))
        return false;
        
    int choice;
    Chooser* chooser = this->getChooser();
    //FIXME:if no chooser, consider providing the largest color image
    //what are the odds that the largest image would be monochrome?
    if (NULL == chooser) {
        choice = 0;
    } else {
        chooser->begin(count);
        for (int i = 0; i < count; i++)
        {
            //need to find out the config, width, and height from the stream
            int width = readByte(buf, 6 + i*16);
            int height = readByte(buf, 7 + i*16);
            int offset = read4Bytes(buf, 18 + i*16);
            int bitCount = read2Bytes(buf, offset+14);
            SkBitmap::Config c;
            //currently only provide ARGB_8888_, but maybe we want kIndex8_Config for 1 and 4, and possibly 8?
            //or maybe we'll determine this based on the provided config
            switch (bitCount)
            {
                case 1:
                case 4:
                    // In reality, at least for the moment, these will be decoded into kARGB_8888 bitmaps.
                    // However, this will be used to distinguish between the lower quality 1bpp and 4 bpp 
                    // images and the higher quality images.
                    c = SkBitmap::kIndex8_Config;
                    break;
                case 8:
                case 24:
                case 32:
                    c = SkBitmap::kARGB_8888_Config;
                    break;
                default:
                    SkDEBUGF(("Image with %ibpp not supported\n", bitCount));
                    continue;
            }
            chooser->inspect(i, c, width, height);
        }
        choice = chooser->choose();
    }
    
    //you never know what the chooser is going to supply
    if (choice >= count || choice < 0)       
        return false;
    
    //skip ahead to the correct header
    //commented out lines are not used, but if i switch to other read method, need to know how many to skip
    //otherwise, they could be used for error checking
    int w = readByte(buf, 6 + choice*16);
    int h = readByte(buf, 7 + choice*16);
    int colorCount = readByte(buf, 8 + choice*16);
    //int reservedToo = readByte(buf, 9 + choice*16);   //0
    //int planes = read2Bytes(buf, 10 + choice*16);       //1 - but often 0
    //int fakeBitCount = read2Bytes(buf, 12 + choice*16); //should be real - usually 0
    int size = read4Bytes(buf, 14 + choice*16);           //matters?
    int offset = read4Bytes(buf, 18 + choice*16);
    if ((size_t)(offset + size) > length)
        return false;
    //int infoSize = read4Bytes(buf, offset);             //40
    //int width = read4Bytes(buf, offset+4);              //should == w
    //int height = read4Bytes(buf, offset+8);             //should == 2*h
    //int planesToo = read2Bytes(buf, offset+12);         //should == 1 (does it?)
    int bitCount = read2Bytes(buf, offset+14);
    
    void (*placePixel)(const int pixelNo, const unsigned char* buf, 
        const int xorOffset, int& x, int y, const int w, 
        SkBitmap* bm, int alphaByte, int m, int shift, SkPMColor* colors) = NULL;
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
            return false;
    }
        
    //these should all be zero, but perhaps are not - need to check
    //int compression = read4Bytes(buf, offset+16);       //0
    //int imageSize = read4Bytes(buf, offset+20);         //0 - sometimes has a value
    //int xPixels = read4Bytes(buf, offset+24);           //0
    //int yPixels = read4Bytes(buf, offset+28);           //0
    //int colorsUsed = read4Bytes(buf, offset+32)         //0 - might have an actual value though
    //int colorsImportant = read4Bytes(buf, offset+36);   //0
        
    int begin = offset + 40;
    //this array represents the colortable
    //if i allow other types of bitmaps, it may actually be used as a part of the bitmap
    SkPMColor* colors = NULL;
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

    bm->setConfig(SkBitmap::kARGB_8888_Config, w, h, calculateRowBytesFor8888(w, bitCount));
    
    if (SkImageDecoder::kDecodeBounds_Mode == mode) {
        delete[] colors;
        return true;
    }

    if (!this->allocPixelRef(bm, NULL))
    {
        delete[] colors;
        return false;
    }
    
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
    return true;
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

#include "SkTRegistry.h"

SkImageDecoder* sk_libico_dfactory(SkStream* stream) {
    // Check to see if the first four bytes are 0,0,1,0
    // FIXME: Is that required and sufficient?
    SkAutoMalloc autoMal(4);
    unsigned char* buf = (unsigned char*)autoMal.get();
    stream->read((void*)buf, 4);
    int reserved = read2Bytes(buf, 0);
    int type = read2Bytes(buf, 2);
    if (reserved != 0 || type != 1) {
        // This stream does not represent an ICO image.
        return NULL;
    }
    return SkNEW(SkICOImageDecoder);
}

static SkTRegistry<SkImageDecoder*, SkStream*> gReg(sk_libico_dfactory);

