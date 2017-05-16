//
// Copyright(c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// angle_deqp_libtester.h:
//   Exports for the ANGLE dEQP libtester module.

#ifndef ANGLE_DEQP_LIBTESTER_H_
#define ANGLE_DEQP_LIBTESTER_H_

#if defined(_WIN32)
#   if defined(ANGLE_DEQP_LIBTESTER_IMPLEMENTATION)
#       define ANGLE_LIBTESTER_EXPORT __declspec(dllexport)
#   else
#       define ANGLE_LIBTESTER_EXPORT __declspec(dllimport)
#   endif
#elif defined(__GNUC__)
#   if defined(ANGLE_DEQP_LIBTESTER_IMPLEMENTATION)
#       define ANGLE_LIBTESTER_EXPORT __attribute__((visibility ("default")))
#   else
#       define ANGLE_LIBTESTER_EXPORT
#   endif
#else
#   define ANGLE_LIBTESTER_EXPORT
#endif

// Exported to the tester app.
ANGLE_LIBTESTER_EXPORT int deqp_libtester_main(int argc, const char *argv[]);
ANGLE_LIBTESTER_EXPORT void deqp_libtester_shutdown_platform();
ANGLE_LIBTESTER_EXPORT bool deqp_libtester_run(const char *caseName);

#endif // ANGLE_DEQP_LIBTESTER_H_
