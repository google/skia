/*#pragma settings NoInline*/

in fragmentProcessor fp;

half4 func1();
half4 func2();
half4 func3();

half4 func1() {
    return func2();
}

half4 func2() {
    return func3();
}

half4 func3() {
    return sample(fp);
}

void main() {
    sk_OutColor = func1();
}
