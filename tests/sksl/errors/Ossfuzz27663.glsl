### Compilation failed:

error: 1: division by zero
void main() { sk_FragColor = float4(1) / - -half4(0); }
                             ^^^^^^^^^^^^^^^^^^^^^^^
error: 1: type mismatch: '=' cannot operate on 'half4', 'float4'
void main() { sk_FragColor = float4(1) / - -half4(0); }
              ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
2 errors
