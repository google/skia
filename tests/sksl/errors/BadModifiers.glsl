### Compilation failed:

error: 1: 'sk_has_side_effects' is not permitted here
const in out uniform flat noperspective sk_has_side_effects inline noinline void func1() {}
                                        ^^^^^^^^^^^^^^^^^^^
error: 1: 'const' is not permitted here
const in out uniform flat noperspective sk_has_side_effects inline noinline void func1() {}
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 1: 'in' is not permitted here
const in out uniform flat noperspective sk_has_side_effects inline noinline void func1() {}
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 1: 'out' is not permitted here
const in out uniform flat noperspective sk_has_side_effects inline noinline void func1() {}
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 1: 'uniform' is not permitted here
const in out uniform flat noperspective sk_has_side_effects inline noinline void func1() {}
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 1: 'flat' is not permitted here
const in out uniform flat noperspective sk_has_side_effects inline noinline void func1() {}
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 1: 'noperspective' is not permitted here
const in out uniform flat noperspective sk_has_side_effects inline noinline void func1() {}
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 1: functions cannot be both 'inline' and 'noinline'
const in out uniform flat noperspective sk_has_side_effects inline noinline void func1() {}
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 3: 'sk_has_side_effects' is not permitted here
void func2(const in out uniform flat noperspective sk_has_side_effects
                                                   ^^^^^^^^^^^^^^^^^^^
error: 3: 'uniform' is not permitted here
void func2(const in out uniform flat noperspective sk_has_side_effects
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 3: 'flat' is not permitted here
void func2(const in out uniform flat noperspective sk_has_side_effects
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 3: 'noperspective' is not permitted here
void func2(const in out uniform flat noperspective sk_has_side_effects
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 3: 'sk_has_side_effects' is not permitted here
void func2(const in out uniform flat noperspective sk_has_side_effects
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 3: 'inline' is not permitted here
void func2(const in out uniform flat noperspective sk_has_side_effects
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 3: 'noinline' is not permitted here
void func2(const in out uniform flat noperspective sk_has_side_effects
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 6: 'sk_has_side_effects' is not permitted here
const in out uniform flat noperspective sk_has_side_effects inline noinline float var;
                                        ^^^^^^^^^^^^^^^^^^^
error: 6: 'in uniform' variables not permitted
const in out uniform flat noperspective sk_has_side_effects inline noinline float var;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 6: 'inline' is not permitted here
const in out uniform flat noperspective sk_has_side_effects inline noinline float var;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 6: 'noinline' is not permitted here
const in out uniform flat noperspective sk_has_side_effects inline noinline float var;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 6: 'const' variables must be initialized
const in out uniform flat noperspective sk_has_side_effects inline noinline float var;
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
20 errors
