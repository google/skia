/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkNoncopyable_DEFINED
#define SkNoncopyable_DEFINED

/** \class SkNoncopyable

SkNoncopyable is the base class for objects that do not want to
be copied. It hides its copy-constructor and its assignment-operator.
*/
class SkNoncopyable {
public:
    SkNoncopyable() = default;

    SkNoncopyable(SkNoncopyable&&) = default;
    SkNoncopyable& operator =(SkNoncopyable&&) = default;

    SkNoncopyable(const SkNoncopyable&) = delete;
    SkNoncopyable& operator=(const SkNoncopyable&) = delete;
};

#endif  // SkNoncopyable_DEFINED
