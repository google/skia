### Compilation failed:

error: 5: 'const' variable initializer must be a constant expression
    const float val = pow(1e38, 2);
                      ^^^^^^^^^^^^
error: 6: unknown identifier 'val'
    float4 y = val.xxxx;
               ^^^
error: 8: unknown identifier 'y'
    return (y.x == val) ? colorGreen : colorRed;
            ^
error: 8: unknown identifier 'val'
    return (y.x == val) ? colorGreen : colorRed;
                   ^^^
4 errors
