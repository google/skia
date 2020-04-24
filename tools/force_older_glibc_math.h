// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#pragma once

// GLIBC_2.27 has new implementations of expf, powf, log2f, exp2f that are pretty nifty,
// but that's not very helpful if you're using an older glibc that doesn't ship those.
__asm__(".symver expf,expf@GLIBC_2.4");
__asm__(".symver powf,powf@GLIBC_2.4");
__asm__(".symver log2f,log2f@GLIBC_2.4");
__asm__(".symver exp2f,exp2f@GLIBC_2.4");
