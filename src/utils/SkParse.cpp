/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTo.h"
#include "include/utils/SkParse.h"

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <string>

static inline bool is_between(int c, int min, int max)
{
    return (unsigned)(c - min) <= (unsigned)(max - min);
}

static inline bool is_ws(int c)
{
    return is_between(c, 1, 32);
}

static inline bool is_digit(int c)
{
    return is_between(c, '0', '9');
}

static inline bool is_sep(int c)
{
    return is_ws(c) || c == ',' || c == ';';
}

static int to_hex(int c)
{
    if (is_digit(c))
        return c - '0';

    c |= 0x20;  // make us lower-case
    if (is_between(c, 'a', 'f'))
        return c + 10 - 'a';
    else
        return -1;
}

static inline bool is_hex(int c)
{
    return to_hex(c) >= 0;
}

static const char* skip_ws(const char str[])
{
    SkASSERT(str);
    while (is_ws(*str))
        str++;
    return str;
}

static const char* skip_sep(const char str[])
{
    SkASSERT(str);
    while (is_sep(*str))
        str++;
    return str;
}

int SkParse::Count(const char str[])
{
    char c;
    int count = 0;
    goto skipLeading;
    do {
        count++;
        do {
            if ((c = *str++) == '\0')
                goto goHome;
        } while (is_sep(c) == false);
skipLeading:
        do {
            if ((c = *str++) == '\0')
                goto goHome;
        } while (is_sep(c));
    } while (true);
goHome:
    return count;
}

int SkParse::Count(const char str[], char separator)
{
    char c;
    int count = 0;
    goto skipLeading;
    do {
        count++;
        do {
            if ((c = *str++) == '\0')
                goto goHome;
        } while (c != separator);
skipLeading:
        do {
            if ((c = *str++) == '\0')
                goto goHome;
        } while (c == separator);
    } while (true);
goHome:
    return count;
}

const char* SkParse::FindHex(const char str[], uint32_t* value)
{
    SkASSERT(str);
    str = skip_ws(str);

    if (!is_hex(*str))
        return nullptr;

    uint32_t n = 0;
    int max_digits = 8;
    int digit;

    while ((digit = to_hex(*str)) >= 0)
    {
        if (--max_digits < 0)
            return nullptr;
        n = (n << 4) | digit;
        str += 1;
    }

    if (*str == 0 || is_ws(*str))
    {
        if (value)
            *value = n;
        return str;
    }
    return nullptr;
}

const char* SkParse::FindS32(const char str[], int32_t* value)
{
    SkASSERT(str);
    str = skip_ws(str);

    int sign = 1;
    int64_t maxAbsValue = std::numeric_limits<int>::max();
    if (*str == '-')
    {
        sign = -1;
        maxAbsValue = -static_cast<int64_t>(std::numeric_limits<int>::min());
        str += 1;
    }

    if (!is_digit(*str)) {
        return nullptr;
    }

    int64_t n = 0;
    while (is_digit(*str))
    {
        n = 10*n + *str - '0';
        if (n > maxAbsValue) {
            return nullptr;
        }

        str += 1;
    }
    if (value) {
        *value = SkToS32(sign*n);
    }
    return str;
}

const char* SkParse::FindScalar(const char str[], SkScalar* value) {
    SkASSERT(str);
    str = skip_ws(str);

    char* stop;
    float v = (float)strtod(str, &stop);
    if (str == stop) {
        return nullptr;
    }
    if (value) {
        *value = v;
    }
    return stop;
}

const char* SkParse::FindScalars(const char str[], SkScalar value[], int count)
{
    SkASSERT(count >= 0);

    if (count > 0)
    {
        for (;;)
        {
            str = SkParse::FindScalar(str, value);
            if (--count == 0 || str == nullptr)
                break;

            // keep going
            str = skip_sep(str);
            if (value)
                value += 1;
        }
    }
    return str;
}

static bool lookup_str(const char str[], const char** table, int count)
{
    while (--count >= 0)
        if (!strcmp(str, table[count]))
            return true;
    return false;
}

bool SkParse::FindBool(const char str[], bool* value)
{
    static const char* gYes[] = { "yes", "1", "true" };
    static const char* gNo[] = { "no", "0", "false" };

    if (lookup_str(str, gYes, std::size(gYes)))
    {
        if (value) *value = true;
        return true;
    }
    else if (lookup_str(str, gNo, std::size(gNo)))
    {
        if (value) *value = false;
        return true;
    }
    return false;
}

int SkParse::FindList(const char target[], const char list[])
{
    size_t  len = strlen(target);
    int     index = 0;

    for (;;)
    {
        const char* end = strchr(list, ',');
        size_t      entryLen;

        if (end == nullptr) // last entry
            entryLen = strlen(list);
        else
            entryLen = end - list;

        if (entryLen == len && memcmp(target, list, len) == 0)
            return index;
        if (end == nullptr)
            break;

        list = end + 1; // skip the ','
        index += 1;
    }
    return -1;
}
