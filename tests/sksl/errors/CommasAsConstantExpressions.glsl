### Compilation failed:

error: 1: array size must be an integer
int a[1, 2];
      ^^^^
error: 2: array size must be an integer
int b[(3, 4)];
      ^^^^^^
error: 3: 'const' variable initializer must be a constant expression
const int d = (5, 6);
              ^^^^^^
3 errors
