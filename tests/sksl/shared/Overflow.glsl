
out vec4 sk_FragColor;
uniform vec4 colorGreen;
vec4 main() {
    float huge = (((((((((9.000001e+35 * 1e+09) * 1e+09) * 1e+09) * 1e+09) * 1e+09) * 1e+09) * 1e+09) * 1e+09) * 1e+09) * 1e+09;
    int hugeI = int((((((((((((((((((((1073741824 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2);
    uint hugeU = ((((((((((((((((((2147483648u * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u;
    int hugeS = ((((((((((((((((16384 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2;
    uint hugeUS = (((((((((((((((32768u * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u;
    int hugeNI = int(((((((((((((((((((-2147483648 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2);
    int hugeNS = (((((((((((((((-32768 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2;
    const ivec4 i4 = ivec4(2);
    ivec4 hugeIvec = ((((((((((((((ivec4(1073741824) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4;
    const uvec4 u4 = uvec4(2u);
    uvec4 hugeUvec = (((((((((((((uvec4(2147483648u) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4;
    return ((((((((colorGreen * clamp(huge, 0.0, 1.0)) * clamp(float(hugeI), 0.0, 1.0)) * clamp(float(hugeU), 0.0, 1.0)) * clamp(float(hugeS), 0.0, 1.0)) * clamp(float(hugeUS), 0.0, 1.0)) * clamp(float(hugeNI), 0.0, 1.0)) * clamp(float(hugeNS), 0.0, 1.0)) * clamp(vec4(hugeIvec), 0.0, 1.0)) * clamp(vec4(hugeUvec), 0.0, 1.0);
}
