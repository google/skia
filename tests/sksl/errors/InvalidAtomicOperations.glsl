### Compilation failed:

error: 18: type mismatch: '=' cannot operate on 'atomicUint', 'int'
    wgCounterA = 1;
    ^^^^^^^^^^^^^^
error: 19: cannot construct 'atomicUint'
    wgCounterA = atomicUint(1);
                 ^^^^^^^^^^^^^
error: 21: '+' cannot operate on 'atomicUint'
    +wgCounterA;
    ^^^^^^^^^^^
error: 22: '-' cannot operate on 'atomicUint'
    -wgCounterA;
    ^^^^^^^^^^^
error: 23: '!' cannot operate on 'atomicUint'
    !wgCounterA;
    ^^^^^^^^^^^
error: 24: '~' cannot operate on 'atomicUint'
    ~wgCounterA;
    ^^^^^^^^^^^
error: 26: '++' cannot operate on 'atomicUint'
    wgCounterA++;
    ^^^^^^^^^^^^
error: 27: '--' cannot operate on 'atomicUint'
    wgCounterA--;
    ^^^^^^^^^^^^
error: 28: '++' cannot operate on 'atomicUint'
    ++wgCounterA;
    ^^^^^^^^^^^^
error: 29: '--' cannot operate on 'atomicUint'
    --wgCounterA;
    ^^^^^^^^^^^^
error: 31: '-' cannot operate on 'atomicUint'
    wgCounterA = -wgCounterA;
                 ^^^^^^^^^^^
error: 32: '+' cannot operate on 'atomicUint'
    wgCounterA = +wgCounterA;
                 ^^^^^^^^^^^
error: 33: assignments to opaque type 'atomicUint' are not permitted
    wgCounterA = wgCounterB;
    ^^^^^^^^^^^^^^^^^^^^^^^
error: 34: type mismatch: '+=' cannot operate on 'atomicUint', 'atomicUint'
    wgCounterA += wgCounterB;
    ^^^^^^^^^^^^^^^^^^^^^^^^
error: 35: type mismatch: '-=' cannot operate on 'atomicUint', 'atomicUint'
    wgCounterA -= wgCounterB;
    ^^^^^^^^^^^^^^^^^^^^^^^^
error: 36: type mismatch: '*=' cannot operate on 'atomicUint', 'atomicUint'
    wgCounterA *= wgCounterB;
    ^^^^^^^^^^^^^^^^^^^^^^^^
error: 37: type mismatch: '/=' cannot operate on 'atomicUint', 'atomicUint'
    wgCounterA /= wgCounterB;
    ^^^^^^^^^^^^^^^^^^^^^^^^
error: 39: type mismatch: '+' cannot operate on 'atomicUint', 'int'
    wgCounterA = wgCounterA + 1;
                 ^^^^^^^^^^^^^^
error: 40: type mismatch: '-' cannot operate on 'atomicUint', 'int'
    wgCounterA = wgCounterA - 1;
                 ^^^^^^^^^^^^^^
error: 41: type mismatch: '*' cannot operate on 'atomicUint', 'int'
    wgCounterA = wgCounterA * 1;
                 ^^^^^^^^^^^^^^
error: 42: type mismatch: '/' cannot operate on 'atomicUint', 'int'
    wgCounterA = wgCounterA / 1;
                 ^^^^^^^^^^^^^^
error: 43: type mismatch: '+' cannot operate on 'atomicUint', 'atomicUint'
    wgCounterA = wgCounterA + wgCounterB;
                 ^^^^^^^^^^^^^^^^^^^^^^^
error: 44: type mismatch: '-' cannot operate on 'atomicUint', 'atomicUint'
    wgCounterA = wgCounterA - wgCounterB;
                 ^^^^^^^^^^^^^^^^^^^^^^^
error: 45: type mismatch: '*' cannot operate on 'atomicUint', 'atomicUint'
    wgCounterA = wgCounterA * wgCounterB;
                 ^^^^^^^^^^^^^^^^^^^^^^^
error: 46: type mismatch: '/' cannot operate on 'atomicUint', 'atomicUint'
    wgCounterA = wgCounterA / wgCounterB;
                 ^^^^^^^^^^^^^^^^^^^^^^^
error: 48: type mismatch: '==' cannot operate on 'atomicUint', 'atomicUint'
    wgCounterA == wgCounterB;
    ^^^^^^^^^^^^^^^^^^^^^^^^
error: 49: type mismatch: '!=' cannot operate on 'atomicUint', 'atomicUint'
    wgCounterA != wgCounterB;
    ^^^^^^^^^^^^^^^^^^^^^^^^
error: 50: type mismatch: '<' cannot operate on 'atomicUint', 'atomicUint'
    wgCounterA < wgCounterB;
    ^^^^^^^^^^^^^^^^^^^^^^^
error: 51: type mismatch: '<=' cannot operate on 'atomicUint', 'atomicUint'
    wgCounterA <= wgCounterB;
    ^^^^^^^^^^^^^^^^^^^^^^^^
error: 52: type mismatch: '>' cannot operate on 'atomicUint', 'atomicUint'
    wgCounterA > wgCounterB;
    ^^^^^^^^^^^^^^^^^^^^^^^
error: 53: type mismatch: '>=' cannot operate on 'atomicUint', 'atomicUint'
    wgCounterA >= wgCounterB;
    ^^^^^^^^^^^^^^^^^^^^^^^^
error: 54: type mismatch: '&&' cannot operate on 'atomicUint', 'atomicUint'
    wgCounterA && wgCounterB;
    ^^^^^^^^^^^^^^^^^^^^^^^^
error: 55: type mismatch: '||' cannot operate on 'atomicUint', 'atomicUint'
    wgCounterA || wgCounterB;
    ^^^^^^^^^^^^^^^^^^^^^^^^
error: 56: type mismatch: '&' cannot operate on 'atomicUint', 'atomicUint'
    wgCounterA & wgCounterB;
    ^^^^^^^^^^^^^^^^^^^^^^^
error: 57: type mismatch: '|' cannot operate on 'atomicUint', 'atomicUint'
    wgCounterA | wgCounterB;
    ^^^^^^^^^^^^^^^^^^^^^^^
error: 59: expected 'uint', but found 'atomicUint'
    uint a = wgCounterA;
             ^^^^^^^^^^
error: 60: construction of struct type 'S' with atomic member is not allowed
    wgStructWithAtomicMember = S(1);
                               ^^^^
error: 61: cannot construct 'atomicUint'
    wgStructWithAtomicMember = S(atomicUint(1));
                                 ^^^^^^^^^^^^^
error: 61: construction of struct type 'S' with atomic member is not allowed
    wgStructWithAtomicMember = S(atomicUint(1));
                               ^^^^^^^^^^^^^^^^
error: 62: construction of struct type 'S' with atomic member is not allowed
    wgStructWithAtomicMember = S(wgCounterA);
                               ^^^^^^^^^^^^^
error: 64: assignments to opaque type 'atomicUint' are not permitted
    wgAtomicArray[0] = wgCounterA;
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 65: assignments to opaque type 'atomicUint' are not permitted
    wgAtomicArray[1] = wgCounterB;
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 66: assignments to opaque type 'atomicUint[2]' are not permitted
    wgAtomicArray = wgAtomicArray2;
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 67: construction of array type 'atomicUint[2]' with atomic member is not allowed
    wgAtomicArray = atomicUint[2](wgCounterA, wgCounterB);
                    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 69: assignments to opaque type 'S' are not permitted
    wgStructWithAtomicMemberArray[0] = wgStructWithAtomicMember;
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 70: construction of array type 'S[2]' with atomic member is not allowed
    wgStructWithAtomicMemberArray = S[2](wgStructWithAtomicMember,
                                    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 72: construction of struct type 'NestedS' with atomic member is not allowed
    wgNestedStructWithAtomicMember = NestedS(wgStructWithAtomicMember);
                                     ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
47 errors
