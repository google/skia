
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bvec3 returns_bool3() {
    return bvec3(true);
}
bvec4 returns_bool4() {
    return bvec4(true);
}
int returns_int() {
    return 1;
}
ivec2 returns_int2() {
    return ivec2(2);
}
ivec3 returns_int3() {
    return ivec3(3);
}
ivec4 returns_int4() {
    return ivec4(4);
}
vec4 main() {
    return (((((bvec2(true) == bvec2(true) && bvec3(true) == returns_bool3()) && bvec4(true) == returns_bool4()) && 1 == returns_int()) && ivec2(2) == returns_int2()) && ivec3(3) == returns_int3()) && ivec4(4) == returns_int4() ? colorGreen : colorRed;
















}
