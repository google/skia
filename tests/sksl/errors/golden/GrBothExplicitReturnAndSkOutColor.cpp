### Compilation failed:

half4 main() {
    sk_OutColor = half4(1, 0, 1, 0);
    return half4(0, 1, 0, 1);
}


### Errors:
error: 3: Fragment processors must not mix sk_OutColor and return statements

1 error
