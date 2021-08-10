
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
struct S {
    int x;
    int y;
};
vec4 main() {
    float f1[4] = float[4](1.0, 2.0, 3.0, 4.0);
    float f2[4] = float[4](1.0, 2.0, 3.0, 4.0);
    float f3[4] = float[4](1.0, 2.0, 3.0, -4.0);
    ivec3 v1[2] = ivec3[2](ivec3(1, 2, 3), ivec3(4, 5, 6));
    ivec3 v2[2] = ivec3[2](ivec3(1, 2, 3), ivec3(4, 5, 6));
    ivec3 v3[2] = ivec3[2](ivec3(1, 2, 3), ivec3(4, 5, -6));
    mat2 m1[3] = mat2[3](mat2(1.0), mat2(2.0), mat2(3.0, 4.0, 5.0, 6.0));
    mat2 m2[3] = mat2[3](mat2(1.0), mat2(2.0), mat2(3.0, 4.0, 5.0, 6.0));
    mat2 m3[3] = mat2[3](mat2(1.0), mat2(2.0, 3.0, 4.0, 5.0), mat2(6.0));
    S s1[3] = S[3](S(1, 2), S(3, 4), S(5, 6));
    S s2[3] = S[3](S(1, 2), S(0, 0), S(5, 6));
    S s3[3] = S[3](S(1, 2), S(3, 4), S(5, 6));
    return ((((((f1 == f2 && f1 != f3) && v1 == v2) && v1 != v3) && m1 == m2) && m1 != m3) && s1 != s2) && s3 == s1 ? colorGreen : colorRed;
}
