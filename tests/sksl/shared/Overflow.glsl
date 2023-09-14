
out vec4 sk_FragColor;
uniform vec4 colorGreen;
vec4 main() {
    const float h = 1e+09;
    float hugeH = ((((((((((1e+36 * h) * h) * h) * h) * h) * h) * h) * h) * h) * h) * h;
    const float f = 1e+09;
    float hugeF = ((((((((((1e+36 * f) * f) * f) * f) * f) * f) * f) * f) * f) * f) * f;
    int hugeI = (((((((((((((((((((1073741824 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2;
    uint hugeU = ((((((((((((((((((2147483648u * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u;
    int hugeS = ((((((((((((((((16384 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2;
    uint hugeUS = (((((((((((((((32768u * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u;
    int hugeNI = ((((((((((((((((((-2147483648 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2;
    int hugeNS = (((((((((((((((-32768 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2;
    const ivec4 i4 = ivec4(2);
    ivec4 hugeIvec = ((((((((((((((ivec4(1073741824) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4;
    const uvec4 u4 = uvec4(2u);
    uvec4 hugeUvec = (((((((((((((uvec4(2147483648u) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4;
    mat4 hugeMxM = mat4(1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20) * mat4(1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20);
    vec4 hugeMxV = mat4(1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20) * vec4(1e+20);
    vec4 hugeVxM = vec4(1e+20) * mat4(1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20, 1e+20);
    return ((((((((((((colorGreen * clamp(hugeH, 0.0, 1.0)) * clamp(hugeF, 0.0, 1.0)) * clamp(float(hugeI), 0.0, 1.0)) * clamp(float(hugeU), 0.0, 1.0)) * clamp(float(hugeS), 0.0, 1.0)) * clamp(float(hugeUS), 0.0, 1.0)) * clamp(float(hugeNI), 0.0, 1.0)) * clamp(float(hugeNS), 0.0, 1.0)) * clamp(vec4(hugeIvec), 0.0, 1.0)) * clamp(vec4(hugeUvec), 0.0, 1.0)) * clamp(hugeMxM[0], 0.0, 1.0)) * clamp(hugeMxV, 0.0, 1.0)) * clamp(hugeVxM, 0.0, 1.0);
}
