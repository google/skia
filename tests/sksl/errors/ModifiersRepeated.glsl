### Compilation failed:

error: 1: 'const' appears more than once
const const float a = 0;
      ^^^^^
error: 2: 'uniform' appears more than once
uniform flat uniform int b;
             ^^^^^^^
error: 3: 'flat' appears more than once
uniform flat noperspective flat noperspective uniform half4 c;
                           ^^^^
error: 3: 'noperspective' appears more than once
uniform flat noperspective flat noperspective uniform half4 c;
                                ^^^^^^^^^^^^^
error: 3: 'uniform' appears more than once
uniform flat noperspective flat noperspective uniform half4 c;
                                              ^^^^^^^
error: 4: 'in' appears more than once
in inout float d;
   ^^^^^
error: 5: 'out' appears more than once
inout out half e;
      ^^^
error: 6: 'inout' appears more than once
inout inout int f;
      ^^^^^
8 errors
