### Compilation failed:

error: 4: atomics are only permitted in workgroup variables and writable storage blocks
atomicUint globalAtomic;          // invalid
^^^^^^^^^^^^^^^^^^^^^^^
error: 5: atomics are only permitted in workgroup variables and writable storage blocks
atomicUint globalAtomicArray[2];  // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 20: atomics are only permitted in workgroup variables and writable storage blocks
S globalStructWithAtomicMember;              // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 21: atomics are only permitted in workgroup variables and writable storage blocks
S globalStructWithAtomicMemberArray[2];      // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 22: atomics are only permitted in workgroup variables and writable storage blocks
NestedS globalStructWithNestedAtomicMember;  // invalid
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 32: variables of type 'atomicUint' may not be uniform
    atomicUint uboAtomic;                    // invalid
    ^^^^^^^^^^^^^^^^^^^^^
error: 31: caused by:
layout(metal, binding = 1) uniform ubo1 {
                                   ^^^^
error: 31: atomics are only permitted in workgroup variables and writable storage blocks
layout(metal, binding = 1) uniform ubo1 {
                                   ^^^^
error: 35: variables of type 'atomicUint[2]' may not be uniform
    atomicUint uboAtomicArray[2];            // invalid
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 34: caused by:
layout(metal, binding = 2) uniform ubo2 {
                                   ^^^^
error: 34: atomics are only permitted in workgroup variables and writable storage blocks
layout(metal, binding = 2) uniform ubo2 {
                                   ^^^^
error: 8: variables of type 'atomicUint' may not be uniform
    atomicUint structMemberAtomic;          // valid
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 37: caused by:
layout(metal, binding = 3) uniform ubo3 {
                                   ^^^^
error: 37: atomics are only permitted in workgroup variables and writable storage blocks
layout(metal, binding = 3) uniform ubo3 {
                                   ^^^^
error: 40: atomics are only permitted in workgroup variables and writable storage blocks
layout(metal, binding = 4) uniform ubo4 {
                                   ^^^^
error: 8: variables of type 'atomicUint' may not be uniform
    atomicUint structMemberAtomic;          // valid
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 43: caused by:
layout(metal, binding = 5) uniform ubo5 {
                                   ^^^^
error: 43: atomics are only permitted in workgroup variables and writable storage blocks
layout(metal, binding = 5) uniform ubo5 {
                                   ^^^^
error: 47: atomics are only permitted in workgroup variables and writable storage blocks
layout(metal, binding = 6) readonly buffer roSsbo1 {
                                           ^^^^^^^
error: 50: atomics are only permitted in workgroup variables and writable storage blocks
layout(metal, binding = 7) readonly buffer roSsbo2 {
                                           ^^^^^^^
error: 53: atomics are only permitted in workgroup variables and writable storage blocks
layout(metal, binding = 8) readonly buffer roSsbo3 {
                                           ^^^^^^^
error: 56: atomics are only permitted in workgroup variables and writable storage blocks
layout(metal, binding = 9) readonly buffer roSsbo4 {
                                           ^^^^^^^
error: 59: atomics are only permitted in workgroup variables and writable storage blocks
layout(metal, binding = 10) readonly buffer roSsbo5 {
                                            ^^^^^^^
error: 64: atomics are only permitted in workgroup variables and writable storage blocks
    atomicUint localAtomic;                // invalid
    ^^^^^^^^^^^^^^^^^^^^^^
error: 65: atomics are only permitted in workgroup variables and writable storage blocks
    atomicUint localAtomicArray[2];        // invalid
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 66: atomics are only permitted in workgroup variables and writable storage blocks
    S localStructWithAtomicMember;          // invalid
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 67: atomics are only permitted in workgroup variables and writable storage blocks
    S localStructWithAtomicMemberArray[2];  // invalid
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
27 errors
