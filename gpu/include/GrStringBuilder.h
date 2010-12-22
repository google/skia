/*
    Copyright 2010 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#ifndef GrStringBuilder_DEFINED
#define GrStringBuilder_DEFINED

#include <GrTArray.h>
#include <stdio.h>

// Class used to concat strings together into a single string
// See below for GrSStringBuilder subclass that has a pool of
// stack storage (to avoid malloc).
class GrStringBuilder {
public:
    GrStringBuilder() :
            fChars() {
        fChars.push_back() = '\0';
    }

    GrStringBuilder(const GrStringBuilder& s) :
            fChars(s.fChars) {
        GrAssert('\0' == s.fChars.back());
    }

    GrStringBuilder(const char* s) :
            fChars(s, strlen(s)+1) {
    }

    GrStringBuilder(const GrStringBuilder& a, const GrStringBuilder& b) {
        GrAssert('\0' == a.fChars.back());
        GrAssert('\0' == b.fChars.back());

        fChars.push_back_n(a.fChars.count() + b.fChars.count() - 1);
        char* s = &fChars.front();
        memcpy(s, &a.fChars.front(), a.fChars.count() - 1);
        s += a.fChars.count() - 1;
        memcpy(s, &b.fChars.front(), b.fChars.count());
    }

    GrStringBuilder& operator =(const GrStringBuilder& s) {
        fChars = s.fChars;
        return *this;
    }

    GrStringBuilder& operator =(const char* s) {
        GrAssert('\0' == fChars.back());

        int l = strlen(s);
        fChars.resize_back(l + 1);
        memcpy(&fChars.front(), s, l + 1);
        return *this;
    }

    GrStringBuilder& operator +=(const GrStringBuilder& s) {
        GrAssert('\0' == fChars.back());
        GrAssert('\0' == s.fChars.back());
        fChars.push_back_n(s.length());
        memcpy(&fChars.fromBack(s.length()), &s.fChars.front(), s.fChars.count());
        return *this;
    }

    GrStringBuilder& operator +=(const char* s) {
        GrAssert('\0' == fChars.back());
        int l = strlen(s);
        fChars.push_back_n(l);
        memcpy(&fChars.fromBack(l), s, l + 1);
        return *this;
    }

    GrStringBuilder& operator +=(char c) {
        GrAssert('\0' == fChars.back());
        fChars.back() = c;
        fChars.push_back() = '\0';
        return *this;
    }

    void appendInt(int x) {
        GR_STATIC_ASSERT(4 == sizeof(int));
        // -, 10 digits, null char
        char temp[12];
        sprintf(temp, "%d", x);
        *this += temp;
    }

    char& operator [](int i) {
        GrAssert(i < length());
        return fChars[i];
    }

    char operator [](int i) const {
        GrAssert(i < length());
        return fChars[i];
    }

    const char* cstr() const { return &fChars.front(); }

    int length() const { return fChars.count() - 1; }

protected:
    // helpers for GrSStringBuilder (with storage on the stack)

    GrStringBuilder(void* stackChars, int stackCount) :
            fChars(stackCount ? stackChars : NULL,
                   stackCount) {
        fChars.push_back() = '\0';
    }

    GrStringBuilder(void* stackChars,
                    int stackCount,
                    const GrStringBuilder& s) :
            fChars(s.fChars,
                   (stackCount ? stackChars : NULL),
                   stackCount) {
    }

    GrStringBuilder(void* stackChars,
                    int stackCount,
                    const char* s) :
            fChars(s,
                   strlen(s)+1,
                   stackCount ? stackChars : NULL,
                   stackCount) {
    }

    GrStringBuilder(void* stackChars,
                    int stackCount,
                    const GrStringBuilder& a,
                    const GrStringBuilder& b) :
            fChars(stackCount ? stackChars : NULL,
                   stackCount) {
        GrAssert('\0' == a.fChars.back());
        GrAssert('\0' == b.fChars.back());

        fChars.push_back_n(a.fChars.count() + b.fChars.count() - 1);
        char* s = &fChars.front();
        memcpy(s, &a.fChars.front(), a.fChars.count() - 1);
        s += a.fChars.count() - 1;
        memcpy(s, &b.fChars.front(), b.fChars.count());
    }

private:
    GrTArray<char, true>  fChars;
};

template <int STACK_COUNT = 128>
class GrSStringBuilder : public GrStringBuilder {
public:
    GrSStringBuilder() : GrStringBuilder(fStackChars, STACK_COUNT) {}

    GrSStringBuilder(const GrStringBuilder& s) : GrStringBuilder(fStackChars,
                                                                 STACK_COUNT,
                                                                 s) {
    }

    GrSStringBuilder(const char* s) : GrStringBuilder(fStackChars,
                                                      STACK_COUNT,
                                                      s) {
    }

    GrSStringBuilder(const GrStringBuilder& a, const GrStringBuilder& b) :
            GrStringBuilder(fStackChars, STACK_COUNT, a, b) {
    }
private:
    char fStackChars[STACK_COUNT];
};

#endif

