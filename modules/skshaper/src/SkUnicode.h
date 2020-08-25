/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkUnicode_DEFINED
#define SkUnicode_DEFINED

#include "include/core/SkTypes.h"
#include "src/core/SkSpan.h"
#include "src/utils/SkUTF.h"
#include <vector>

#if !defined(SKUNICODE_IMPLEMENTATION)
    #define SKUNICODE_IMPLEMENTATION 0
#endif

#if !defined(SKUNICODE_API)
    #if defined(SKSHAPER_DLL)
        #if defined(_MSC_VER)
            #if SKUNICODE_IMPLEMENTATION
                #define SKUNICODE_API __declspec(dllexport)
            #else
                #define SKUNICODE_API __declspec(dllimport)
            #endif
        #else
            #define SKUNICODE_API __attribute__((visibility("default")))
        #endif
    #else
        #define SKUNICODE_API
    #endif
#endif

class SKUNICODE_API SkBidiIterator {
public:
    typedef int32_t Position;
    typedef uint8_t Level;
    struct Region {
        Region(Position start, Position end, Level level)
            : start(start), end(end), level(level) { }
        Position start;
        Position end;
        Level level;
    };
    enum Direction {
        kLTR,
        kRTL,
    };
    virtual ~SkBidiIterator() {}
    virtual Position getLength() = 0;
    virtual Level getLevelAt(Position) = 0;
    static void ReorderVisual(const Level runLevels[], int levelsCount, int32_t logicalFromVisual[]);
};

class SKUNICODE_API SkUnicode {
    public:
        typedef uint32_t ScriptID;
        typedef uint32_t CombiningClass;
        typedef uint32_t GeneralCategory;
        enum class TextDirection {
            kLTR,
            kRTL,
        };
        typedef size_t Position;
        typedef uint8_t BidiLevel;
        struct BidiRegion {
            BidiRegion(Position start, Position end, BidiLevel level)
              : start(start), end(end), level(level) { }
            Position start;
            Position end;
            BidiLevel level;
        };
        enum class LineBreakType {
            kSoftLineBreak,
            kHardLineBreak
        };

        enum class UBreakType {
            kWords,
            kGraphemes,
            kLines
        };
        struct LineBreakBefore {
            LineBreakBefore(Position pos, LineBreakType breakType)
              : pos(pos), breakType(breakType) { }
            Position pos;
            LineBreakType breakType;
        };

        virtual ~SkUnicode() = default;

        virtual bool isControl(SkUnichar utf8) = 0;
        virtual bool isWhitespace(SkUnichar utf8) = 0;

        static int convertUtf8ToUtf16(const char* utf8, size_t utf8Units, std::unique_ptr<uint16_t[]>* utf16) {
            int utf16Units = SkUTF::UTF8ToUTF16(nullptr, 0, utf8, utf8Units);
            if (utf16Units < 0) {
                SkDEBUGF("Convert error: Invalid utf8 input");
                return utf16Units;
            }
            *utf16 = std::unique_ptr<uint16_t[]>(new uint16_t[utf16Units]);
            SkDEBUGCODE(int dstLen =) SkUTF::UTF8ToUTF16(utf16->get(), utf16Units, utf8, utf8Units);
            SkASSERT(dstLen == utf16Units);
            return utf16Units;
        }

        static int convertUtf16ToUtf8(const uint16_t* utf16, size_t utf16Units, std::unique_ptr<char[]>* utf8) {
            int utf8Units = SkUTF::UTF16ToUTF8(nullptr, 0, utf16, utf16Units);
            if (utf8Units < 0) {
                SkDEBUGF("Convert error: Invalid utf16 input");
                return utf8Units;
            }
            *utf8 = std::unique_ptr<char[]>(new char[utf8Units]);
            SkDEBUGCODE(int dstLen =) SkUTF::UTF16ToUTF8(utf8->get(), utf8Units, utf16, utf16Units);
            SkASSERT(dstLen == utf8Units);
            return utf8Units;
        }

        // Iterators (used in SkShaper)
        virtual std::unique_ptr<SkBidiIterator> makeBidiIterator
            (const uint16_t text[], int count, SkBidiIterator::Direction) = 0;
        virtual std::unique_ptr<SkBidiIterator> makeBidiIterator
            (const char text[], int count, SkBidiIterator::Direction) = 0;

        // High level methods (that we actually use somewhere=SkParagraph)
        virtual bool getBidiRegions
               (const char utf8[], int utf8Units, TextDirection dir, std::vector<BidiRegion>* results) = 0;
        virtual bool getLineBreaks
               (const char utf8[], int utf8Units, std::vector<LineBreakBefore>* results) = 0;
        virtual bool getWords
               (const char utf8[], int utf8Units, std::vector<Position>* results) = 0;
        virtual bool getGraphemes
               (const char utf8[], int utf8Units, std::vector<Position>* results) = 0;
        virtual bool getWhitespaces
               (const char utf8[], int utf8Units, std::vector<Position>* results) = 0;

        virtual void reorderVisual(const BidiLevel runLevels[], int levelsCount, int32_t logicalFromVisual[]) = 0;

        static std::unique_ptr<SkUnicode> Make();
};

#endif // SkUnicode_DEFINED
