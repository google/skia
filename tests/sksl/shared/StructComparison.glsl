
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform float testArray[5];
struct S {
    int x;
    int y;
    mat2 m;
    float a[5];
};
vec4 main() {
    float array[5] = float[5](1.0, 2.0, 3.0, 4.0, 5.0);
    S s1 = S(1, 2, mat2(1.0), array);
    S s2 = S(1, 2, mat2(1.0), testArray);
    S s3 = S(1, 2, mat2(2.0), float[5](1.0, 2.0, 3.0, 4.0, 5.0));
    return s1 == s2 && s1 != s3 ? colorGreen : colorRed;
}
