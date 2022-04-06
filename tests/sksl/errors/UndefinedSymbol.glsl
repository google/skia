### Compilation failed:

error: 1: unknown identifier 'x'
void a() { x = float2(1); }
           ^
error: 2: unknown identifier 'x'
void b() { float w = x; }
                     ^
error: 3: unknown identifier 'x'
void c() { float w = x, y; }
                     ^
error: 4: unknown identifier 'x'
void d() { float w = x, y = z; }
                     ^
error: 4: unknown identifier 'z'
void d() { float w = x, y = z; }
                            ^
error: 6: unknown identifier 'f'
float e = f, g = h;
          ^
error: 6: unknown identifier 'h'
float e = f, g = h;
                 ^
error: 7: unknown identifier 'j'
float i = j, k;
          ^
8 errors
