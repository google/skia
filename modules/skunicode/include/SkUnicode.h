/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkUnicode_DEFINED
#define SkUnicode_DEFINED
#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/SkBitmaskEnum.h" // IWYU pragma: keep
#include "include/private/SkTArray.h"
#include "src/utils/SkUTF.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#if !defined(SKUNICODE_IMPLEMENTATION)
    #define SKUNICODE_IMPLEMENTATION 0
#endif

#if !defined(SKUNICODE_API)
    #if defined(SKUNICODE_DLL)
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
    virtual ~SkBidiIterator() = default;
    virtual Position getLength() = 0;
    virtual Level getLevelAt(Position) = 0;
};

class SKUNICODE_API SkBreakIterator {
public:
    typedef int32_t Position;
    typedef int32_t Status;
    virtual ~SkBreakIterator() = default;
    virtual Position first() = 0;
    virtual Position current() = 0;
    virtual Position next() = 0;
    virtual Status status() = 0;
    virtual bool isDone() = 0;
    virtual bool setText(const char utftext8[], int utf8Units) = 0;
    virtual bool setText(const char16_t utftext16[], int utf16Units) = 0;
};

class SKUNICODE_API SkUnicode {
    public:
        enum CodeUnitFlags {
            kNoCodeUnitFlag = 0x00,
            kPartOfWhiteSpaceBreak = 0x01,
            kGraphemeStart = 0x02,
            kSoftLineBreakBefore = 0x04,
            kHardLineBreakBefore = 0x08,
            kPartOfIntraWordBreak = 0x10,
            kControl = 0x20,
            kTabulation = 0x40,
        };
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
            kSoftLineBreak = 0,
            kHardLineBreak = 100,
        };

        enum class BreakType {
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

        virtual SkString toUpper(const SkString&) = 0;

        // Methods used in SkShaper and SkText
        virtual std::unique_ptr<SkBidiIterator> makeBidiIterator
            (const uint16_t text[], int count, SkBidiIterator::Direction) = 0;
        virtual std::unique_ptr<SkBidiIterator> makeBidiIterator
            (const char text[], int count, SkBidiIterator::Direction) = 0;
        virtual std::unique_ptr<SkBreakIterator> makeBreakIterator
            (const char locale[], BreakType breakType) = 0;
        virtual std::unique_ptr<SkBreakIterator> makeBreakIterator(BreakType type) = 0;

        // Methods used in SkParagraph
        static bool isTabulation(SkUnicode::CodeUnitFlags flags);
        static bool isHardLineBreak(SkUnicode::CodeUnitFlags flags);
        static bool isSoftLineBreak(SkUnicode::CodeUnitFlags flags);
        static bool isGraphemeStart(SkUnicode::CodeUnitFlags flags);
        static bool isControl(SkUnicode::CodeUnitFlags flags);
        static bool isPartOfWhiteSpaceBreak(SkUnicode::CodeUnitFlags flags);
        virtual bool getBidiRegions(const char utf8[],
                                    int utf8Units,
                                    TextDirection dir,
                                    std::vector<BidiRegion>* results) = 0;
        virtual bool getWords(const char utf8[], int utf8Units, std::vector<Position>* results) = 0;
        virtual bool computeCodeUnitFlags(char utf8[], int utf8Units, bool replaceTabs,
                                      SkTArray<SkUnicode::CodeUnitFlags, true>* results) = 0;
        virtual bool computeCodeUnitFlags(char16_t utf16[], int utf16Units, bool replaceTabs,
                                      SkTArray<SkUnicode::CodeUnitFlags, true>* results) = 0;

        static SkString convertUtf16ToUtf8(const char16_t * utf16, int utf16Units);
        static SkString convertUtf16ToUtf8(const std::u16string& utf16);
        static std::u16string convertUtf8ToUtf16(const char* utf8, int utf8Units);
        static std::u16string convertUtf8ToUtf16(const SkString& utf8);

        template <typename Appender8, typename Appender16>
        bool extractUtfConversionMapping(SkSpan<const char> utf8, Appender8&& appender8, Appender16&& appender16) {
            size_t size8 = 0;
            size_t size16 = 0;
            auto ptr = utf8.begin();
            auto end = utf8.end();
            while (ptr < end) {

                size_t index = ptr - utf8.begin();
                SkUnichar u = SkUTF::NextUTF8(&ptr, end);

                // All UTF8 code units refer to the same codepoint
                size_t next = ptr - utf8.begin();
                for (auto i = index; i < next; ++i) {
                    //fUTF16IndexForUTF8Index.emplace_back(fUTF8IndexForUTF16Index.size());
                    appender16(size8);
                    ++size16;
                }
                //SkASSERT(fUTF16IndexForUTF8Index.size() == next);
                SkASSERT(size16 == next);
                if (size16 != next) {
                    return false;
                }

                // One or two UTF16 code units refer to the same codepoint
                uint16_t buffer[2];
                size_t count = SkUTF::ToUTF16(u, buffer);
                //fUTF8IndexForUTF16Index.emplace_back(index);
                appender8(index);
                ++size8;
                if (count > 1) {
                    //fUTF8IndexForUTF16Index.emplace_back(index);
                    appender8(index);
                    ++size8;
                }
            }
            //fUTF16IndexForUTF8Index.emplace_back(fUTF8IndexForUTF16Index.size());
            appender16(size8);
            ++size16;
            //fUTF8IndexForUTF16Index.emplace_back(fText.size());
            appender8(utf8.size());
            ++size8;

            return true;
        }

        template <typename Callback>
        void forEachCodepoint(const char* utf8, int32_t utf8Units, Callback&& callback) {
            const char* current = utf8;
            const char* end = utf8 + utf8Units;
            while (current < end) {
                auto before = current - utf8;
                SkUnichar unichar = SkUTF::NextUTF8(&current, end);
                if (unichar < 0) unichar = 0xFFFD;
                auto after = current - utf8;
                uint16_t buffer[2];
                size_t count = SkUTF::ToUTF16(unichar, buffer);
                callback(unichar, before, after, count);
            }
        }

        template <typename Callback>
        void forEachCodepoint(const char16_t* utf16, int32_t utf16Units, Callback&& callback) {
            const char16_t* current = utf16;
            const char16_t* end = utf16 + utf16Units;
            while (current < end) {
                auto before = current - utf16;
                SkUnichar unichar = SkUTF::NextUTF16((const uint16_t**)&current, (const uint16_t*)end);
                auto after = current - utf16;
                callback(unichar, before, after);
            }
        }

        template <typename Callback>
        void forEachBidiRegion(const uint16_t utf16[], int utf16Units, SkBidiIterator::Direction dir, Callback&& callback) {
            auto iter = makeBidiIterator(utf16, utf16Units, dir);
            const uint16_t* start16 = utf16;
            const uint16_t* end16 = utf16 + utf16Units;
            SkBidiIterator::Level currentLevel = 0;

            SkBidiIterator::Position pos16 = 0;
            while (pos16 <= iter->getLength()) {
                auto level = iter->getLevelAt(pos16);
                if (pos16 == 0) {
                    currentLevel = level;
                } else if (level != currentLevel) {
                    callback(pos16, start16 - utf16, currentLevel);
                    currentLevel = level;
                }
                if (start16 == end16) {
                    break;
                }
                SkUnichar u = SkUTF::NextUTF16(&start16, end16);
                pos16 += SkUTF::ToUTF16(u);
            }
        }

        template <typename Callback>
        void forEachBreak(const char16_t utf16[], int utf16Units, SkUnicode::BreakType type, Callback&& callback) {
            auto iter = makeBreakIterator(type);
            iter->setText(utf16, utf16Units);
            auto pos = iter->first();
            do {
                callback(pos, iter->status());
                pos = iter->next();
            } while (!iter->isDone());
        }

        virtual void reorderVisual(const BidiLevel runLevels[], int levelsCount, int32_t logicalFromVisual[]) = 0;

        static std::unique_ptr<SkUnicode> Make();

        static std::unique_ptr<SkUnicode> Make(SkSpan<char> text,
                                               std::vector<SkUnicode::BidiRegion> bidiRegions,
                                               std::vector<SkUnicode::Position> words,
                                               std::vector<SkUnicode::Position> graphemeBreaks,
                                               std::vector<SkUnicode::LineBreakBefore> lineBreaks);
};

namespace sknonstd {
    template <> struct is_bitmask_enum<SkUnicode::CodeUnitFlags> : std::true_type {};
}  // namespace sknonstd
#endif // SkUnicode_DEFINED
