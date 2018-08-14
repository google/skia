
FILES
-----

spillage.spir32 -- spir32 dump
gen.xlsx        -- performance plots of a more complicated version of the reproducer
README.txt      -- this file


KERNELS
-------

There are three kernels in the SPIR32 dump with the following signatures:

  spillage_uint(__global const uint* const __restrict__ in, __global uint* const out)
  spillage_ulong_v1(__global const ulong* const __restrict__ in, __global ulong* const out)
  spillage_ulong_v2(__global const ulong* const __restrict__ in, __global ulong* const out)

The "_uint" kernel uses ~100 32-bit uint registers.  There should be a
peak of ~100 "live" registers even in this simplified reproducer.

The two "_long" kernels each use ~50 64-bit ulong registers and only
differ in how they compare registers:

 | //
 | // 64-BIT
 | //
 |   {
 |     const ulong t = min(r1  ,r2  );
 |     r2            = max(r1  ,r2  );
 |     r1            = t;
 |  }
 |
 | //
 | // 64-BIT -- SLIGHTLY FASTER ON BOTH HSW/BDW
 | //
 |   if (r1 > r2) {
 |     const ulong t = r2;
 |     r2            = r1;
 |     r1            = t;
 |   }

The reproducer kernels do not use local memory or barriers.

The production kernels can be tuned by me and are meant to maximally
exploit the 128x8 register file.  

I really need to be able to get the kernels running in a SIMD8 x ~100
32-bit register mode.


WORKGROUPS
----------

These reproducer kernels should be launched with *large* local
workgroup sizes since the production kernels do use shared memory (and
barriers) to minimize load/stores to global memory.

Here are the defaults I've been focusing on:

 |                   NOMINAL      NOMINAL        MAX
 | ARCH  PKG      GLOBAL WG SZ  LOCAL WG SZ  LOCAL WG SZ
 | ----  -------  ------------  -----------  -----------
 | HSW   HD 4600     1120           280          512
 | BDW   HD 6000     2688           224          256


The global workgroup size choice is meant to fully occupy the
package's total number of EUs:  #_of_EUs * 7 * SIMD8.

The kernels will run with any size local workgroup size but it
requires at least 8 to get a coalesced load/store.

The nominal and max workgroup sizes should reveal the issue.

ISSUE 1
-------

As noted on the forums, the 48 EU BDW HD 6000 performance is terrible
beyond, what I'm guessing is, ~64 registers/thread. 

For the same kernel, but a necessarily smaller IGP-occupying
workgroup, the Haswell 20 EU HD 4600 performs _very_ well.

See the spreadsheet for the results of a more complex production (but
not final) kernel.

The _hypothesis_ is that I'm witnessing major spillage in the
Broadwell kernels.  The performance gets worse and worse the larger
the workgroup size.

This behavior isn't observed on Haswell or kernels with a very small
(and semi-useless) number of registers/thread.

ISSUE 2
-------

It's unclear to me if ulong comparison/min/max code generation is
correct or if I'm simply observing issue (1).  It would be great if
you could double-check that 64-bit ulong SEL (min/max/cmp) is optimal.


CONTACT
-------

Call or email if you have questions:

Allan MacKinnon / allanmac@pixel.io / (617) 642-3623

I'm in Sunnyvale, CA for the next few weeks if face to face would help
track this bug down.

Thanks,

-Allan

