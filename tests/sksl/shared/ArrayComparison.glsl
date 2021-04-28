
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
struct S {
    int x;
    int y;
};
vec4 main() {
    float f1[4];
    f1[0] = 1.0;
    f1[1] = 2.0;
    f1[2] = 3.0;
    f1[3] = 4.0;
    float f2[4];
    f2[0] = 1.0;
    f2[1] = 2.0;
    f2[2] = 3.0;
    f2[3] = 4.0;
    float f3[4];
    f3[0] = 1.0;
    f3[1] = 2.0;
    f3[2] = 3.0;
    f3[3] = -4.0;
    S s1[3];
    s1[0] = S(1, 2);
    s1[1] = S(3, 4);
    s1[2] = S(5, 6);
    S s2[3];
    s2[0] = S(1, 2);
    s2[1] = S(0, 0);
    s2[2] = S(5, 6);
    S s3[3];
    s3[0] = S(1, 2);
    s3[1] = S(3, 4);
    s3[2] = S(5, 6);
    return ((f1 == f2 && f1 != f3) && s1 != s2) && s3 == s1 ? colorGreen : colorRed;
}
