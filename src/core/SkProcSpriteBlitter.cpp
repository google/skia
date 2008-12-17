/* libs/graphics/sgl/SkProcSpriteBlitter.cpp
**
** Copyright 2006, The Android Open Source Project
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

#if 0   // experimental

class SkProcSpriteBlitter : public SkSpriteBlitter {
public:
    typedef void (*Proc)(void* dst, const void* src, int count, const SkPMColor ctable[]);

    SkProcSpriteBlitter(const SkBitmap& source, Proc proc, unsigned srcShift, unsigned dstShift)
        : SkSpriteBlitter(source), fProc(proc), fSrcShift(SkToU8(srcShift)), fDstShift(SkToU8(dstShift)) {}

    virtual void blitRect(int x, int y, int width, int height)
    {
        size_t      dstRB = fDevice.rowBytes();
        size_t      srcRB = fSource.rowBytes();
        char*       dst = (char*)fDevice.getPixels() + y * dstRB + (x << fDstShift);
        const char* src = (const char*)fSource.getPixels() + (y - fTop) * srcRB + ((x - fLeft) << fSrcShift);
        Proc        proc = fProc;
        const SkPMColor* ctable = NULL;

        if fSource.getColorTable())
            ctable = fSource.getColorTable()->lockColors();

        while (--height >= 0)
        {
            proc(dst, src, width, ctable);
            dst += dstRB;
            src += srcRB;
        }

        if fSource.getColorTable())
            fSource.getColorTable()->unlockColors(false);
    }

private:
    Proc    fProc;
    uint8_t fSrcShift, fDstShift;
};

#endif
