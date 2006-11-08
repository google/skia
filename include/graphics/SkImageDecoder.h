/* include/graphics/SkImageDecoder.h
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

#ifndef SkImageDecoder_DEFINED
#define SkImageDecoder_DEFINED

#include "SkBitmap.h"
#include "SkBitmapRef.h"
#include "SkRefCnt.h"

class SkStream;

/** \class SkImageDecoder

    Base class for decoding compressed images into a SkBitmap
*/
class SkImageDecoder {
public:
    virtual ~SkImageDecoder();
    
    /** \class Peeker
    
        Base class for optional callbacks to retrive meta/chunk data out of
        an image as it is being decoded.
    */
    class Peeker : public SkRefCnt {
    public:
        /** Return true to continue decoding, or false to indicate an error, which
            will cause the decoder to not return the image.
        */
        virtual bool peek(const char tag[], const void* data, size_t length) = 0;
    };

    Peeker* getPeeker() const { return fPeeker; }
    Peeker* setPeeker(Peeker*);
    
    /** \class Peeker
    
        Base class for optional callbacks to retrive meta/chunk data out of
        an image as it is being decoded.
    */
    class Chooser : public SkRefCnt {
    public:
        virtual void begin(int count) {}
        virtual void inspect(int index, SkBitmap::Config config, int width, int height) {}
        /** Return the index of the subimage you want, or -1 to choose none of them.
        */
        virtual int choose() = 0;
    };

    Chooser* getChooser() const { return fChooser; }
    Chooser* setChooser(Chooser*);

    /** Given a stream, decode it into the specified bitmap.
        If the decoder can decompress the image, it should call setConfig() on the bitmap,
        and then call allocPixels(), which will allocated offscreen memory for the pixels.
        It can then set the pixels with the decompressed image. If the image cannot be
        decompressed, return false and leave the bitmap unchanged.
    */
    virtual bool onDecode(SkStream*, SkBitmap* bitmap, SkBitmap::Config pref) = 0;

    /** Given a stream, this will try to find an appropriate decoder object.
        If none is found, the method returns NULL.
    */
    static SkImageDecoder* Factory(SkStream*);

    /** Decode the image stored in the specified file, and store the result
        in bitmap. Return true for success or false on failure.

        If pref is kNo_Config, then the decoder is free to choose the most natural
        config given the image data. If pref something other than kNo_Config,
        the decoder will attempt to decode the image into that format, unless
        there is a conflict (e.g. the image has per-pixel alpha and the bitmap's
        config does not support that), in which case the decoder will choose a
        closest match configuration.
    */
    static bool DecodeFile(const char file[], SkBitmap* bitmap,
                           SkBitmap::Config pref = SkBitmap::kNo_Config);
    /** Decode the image stored in the specified memory buffer, and store the result
        in bitmap. Return true for success or false on failure.

        If pref is kNo_Config, then the decoder is free to choose the most natural
        config given the image data. If pref something other than kNo_Config,
        the decoder will attempt to decode the image into that format, unless
        there is a conflict (e.g. the image has per-pixel alpha and the bitmap's
        config does not support that), in which case the decoder will choose a
        closest match configuration.
    */
    static bool DecodeMemory(const void* buffer, size_t size, SkBitmap* bitmap,
                             SkBitmap::Config pref = SkBitmap::kNo_Config);
    /** Decode the image stored in the specified SkStream, and store the result
        in bitmap. Return true for success or false on failure.

        If pref is kNo_Config, then the decoder is free to choose the most natural
        config given the image data. If pref something other than kNo_Config,
        the decoder will attempt to decode the image into that format, unless
        there is a conflict (e.g. the image has per-pixel alpha and the bitmap's
        config does not support that), in which case the decoder will choose a
        closest match configuration.
    */
    static bool DecodeStream(SkStream*, SkBitmap* bitmap,
                             SkBitmap::Config pref = SkBitmap::kNo_Config);

    /** Decode the image stored at the specified URL, and store the result
        in bitmap. Return true for success or false on failure. The URL restrictions
        are device dependent. On Win32 and WinCE, the URL may be ftp, http or
        https.

        If pref is kNo_Config, then the decoder is free to choose the most natural
        config given the image data. If pref something other than kNo_Config,
        the decoder will attempt to decode the image into that format, unless
        there is a conflict (e.g. the image has per-pixel alpha and the bitmap's
        config does not support that), in which case the decoder will choose a
        closest match configuration.
    */
    static bool DecodeURL(const char url[], SkBitmap* bitmap,
                          SkBitmap::Config pref = SkBitmap::kNo_Config);

    /** Return the default config for the running device.
        Currently this used as a suggestion to image decoders that need to guess
        what config they should decode into.
        Default is kNo_Config, but this can be changed with SetDeviceConfig()
    */
    static SkBitmap::Config GetDeviceConfig();
    /** Set the default config for the running device.
        Currently this used as a suggestion to image decoders that need to guess
        what config they should decode into.
        Default is kNo_Config.
        This can be queried with GetDeviceConfig()
    */
    static void SetDeviceConfig(SkBitmap::Config);

  /** @cond UNIT_TEST */
    SkDEBUGCODE(static void UnitTest();)
  /** @endcond */

protected:
    SkImageDecoder();

    // helper function for decoders to handle the (common) case where there is only
    // once choice available in the image file.
    bool chooseFromOneChoice(SkBitmap::Config config, int width, int height) const;

private:
    Peeker*  fPeeker;
    Chooser* fChooser;

    // illegal
    SkImageDecoder(const SkImageDecoder&);
    SkImageDecoder& operator=(const SkImageDecoder&);
};

#ifdef SK_SUPPORT_IMAGE_ENCODE

class SkWStream;

class SkImageEncoder {
public:
    enum Type {
        kJPEG_Type,
        kPNG_Type
    };
    static SkImageEncoder* Create(Type);

    virtual ~SkImageEncoder();

    /*  Quality ranges from 0..100 */

    bool encodeFile(const char file[], const SkBitmap&, int quality = 80);
    bool encodeStream(SkWStream*, const SkBitmap&, int quality = 80);

protected:
    virtual bool onEncode(SkWStream*, const SkBitmap&, int quality) = 0;
};

#endif /* SK_SUPPORT_IMAGE_ENCODE */

///////////////////////////////////////////////////////////////////////

#endif

