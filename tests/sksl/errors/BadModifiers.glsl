### Compilation failed:

error: 1: type 'void' does not support qualifier 'readonly writeonly'
const in out uniform flat noperspective $pure inline noinline readonly writeonly pixel_local
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 1: 'const' is not permitted here
const in out uniform flat noperspective $pure inline noinline readonly writeonly pixel_local
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 1: 'in' is not permitted here
const in out uniform flat noperspective $pure inline noinline readonly writeonly pixel_local
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 1: 'out' is not permitted here
const in out uniform flat noperspective $pure inline noinline readonly writeonly pixel_local
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 1: 'uniform' is not permitted here
const in out uniform flat noperspective $pure inline noinline readonly writeonly pixel_local
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 1: 'flat' is not permitted here
const in out uniform flat noperspective $pure inline noinline readonly writeonly pixel_local
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 1: 'noperspective' is not permitted here
const in out uniform flat noperspective $pure inline noinline readonly writeonly pixel_local
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 1: '$pure' is not permitted here
const in out uniform flat noperspective $pure inline noinline readonly writeonly pixel_local
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 1: 'buffer' is not permitted here
const in out uniform flat noperspective $pure inline noinline readonly writeonly pixel_local
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 1: 'pixel_local' is not permitted here
const in out uniform flat noperspective $pure inline noinline readonly writeonly pixel_local
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 1: functions cannot be both 'inline' and 'noinline'
const in out uniform flat noperspective $pure inline noinline readonly writeonly pixel_local
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 4: type 'float' does not support qualifier 'readonly writeonly'
void func2(const in out uniform flat noperspective $pure
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 4: 'uniform' is not permitted here
void func2(const in out uniform flat noperspective $pure
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 4: 'flat' is not permitted here
void func2(const in out uniform flat noperspective $pure
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 4: 'noperspective' is not permitted here
void func2(const in out uniform flat noperspective $pure
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 4: '$pure' is not permitted here
void func2(const in out uniform flat noperspective $pure
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 4: 'inline' is not permitted here
void func2(const in out uniform flat noperspective $pure
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 4: 'noinline' is not permitted here
void func2(const in out uniform flat noperspective $pure
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 4: 'buffer' is not permitted here
void func2(const in out uniform flat noperspective $pure
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 4: 'pixel_local' is not permitted here
void func2(const in out uniform flat noperspective $pure
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 7: type 'float' does not support qualifier 'readonly writeonly'
const in out uniform flat noperspective $pure inline noinline readonly writeonly pixel_local
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 7: 'in uniform' variables not permitted
const in out uniform flat noperspective $pure inline noinline readonly writeonly pixel_local
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 7: 'uniform buffer' variables not permitted
const in out uniform flat noperspective $pure inline noinline readonly writeonly pixel_local
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 7: '$pure' is not permitted here
const in out uniform flat noperspective $pure inline noinline readonly writeonly pixel_local
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 7: 'inline' is not permitted here
const in out uniform flat noperspective $pure inline noinline readonly writeonly pixel_local
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 7: 'noinline' is not permitted here
const in out uniform flat noperspective $pure inline noinline readonly writeonly pixel_local
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 7: 'buffer' is not permitted here
const in out uniform flat noperspective $pure inline noinline readonly writeonly pixel_local
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 7: 'pixel_local' is not permitted here
const in out uniform flat noperspective $pure inline noinline readonly writeonly pixel_local
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 7: 'const' variables must be initialized
const in out uniform flat noperspective $pure inline noinline readonly writeonly pixel_local
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 11: 'in uniform' variables not permitted
buffer MyInterfaceBlock { float var; };
       ^^^^^^^^^^^^^^^^
error: 11: 'readonly' and 'writeonly' qualifiers cannot be combined
buffer MyInterfaceBlock { float var; };
       ^^^^^^^^^^^^^^^^
error: 11: 'uniform buffer' variables not permitted
buffer MyInterfaceBlock { float var; };
       ^^^^^^^^^^^^^^^^
error: 10: '$pure' is not permitted here
const in out uniform flat noperspective $pure inline noinline readonly writeonly pixel_local
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 10: 'inline' is not permitted here
const in out uniform flat noperspective $pure inline noinline readonly writeonly pixel_local
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 10: 'noinline' is not permitted here
const in out uniform flat noperspective $pure inline noinline readonly writeonly pixel_local
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
error: 10: 'pixel_local' is not permitted here
const in out uniform flat noperspective $pure inline noinline readonly writeonly pixel_local
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^...
36 errors
