
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
uniform vec4 colorBlack;
uniform vec4 colorWhite;
uniform vec4 testInputs;
vec4 main() {
    ivec4 intGreen = ivec4(colorGreen * 100.0);
    ivec4 intRed = ivec4(colorRed * 100.0);
    return ((((((((((((((((((((((((((((((mix(intGreen.x, intRed.x, false) == intGreen.x && mix(intGreen.xy, intRed.xy, bvec2(false)) == intGreen.xy) && mix(intGreen.xyz, intRed.xyz, bvec3(false)) == intGreen.xyz) && mix(intGreen, intRed, bvec4(false)) == intGreen) && mix(intGreen.x, intRed.x, true) == intRed.x) && mix(intGreen.xy, intRed.xy, bvec2(true)) == intRed.xy) && mix(intGreen.xyz, intRed.xyz, bvec3(true)) == intRed.xyz) && mix(intGreen, intRed, bvec4(true)) == intRed) && 0 == intGreen.x) && ivec2(0, 100) == intGreen.xy) && ivec3(0, 100, 0) == intGreen.xyz) && ivec4(0, 100, 0, 100) == intGreen) && 100 == intRed.x) && ivec2(100, 0) == intRed.xy) && ivec3(100, 0, 0) == intRed.xyz) && ivec4(100, 0, 0, 100) == intRed) && mix(colorGreen.x, colorRed.x, false) == colorGreen.x) && mix(colorGreen.xy, colorRed.xy, bvec2(false)) == colorGreen.xy) && mix(colorGreen.xyz, colorRed.xyz, bvec3(false)) == colorGreen.xyz) && mix(colorGreen, colorRed, bvec4(false)) == colorGreen) && mix(colorGreen.x, colorRed.x, true) == colorRed.x) && mix(colorGreen.xy, colorRed.xy, bvec2(true)) == colorRed.xy) && mix(colorGreen.xyz, colorRed.xyz, bvec3(true)) == colorRed.xyz) && mix(colorGreen, colorRed, bvec4(true)) == colorRed) && 0.0 == colorGreen.x) && vec2(0.0, 1.0) == colorGreen.xy) && vec3(0.0, 1.0, 0.0) == colorGreen.xyz) && vec4(0.0, 1.0, 0.0, 1.0) == colorGreen) && 1.0 == colorRed.x) && vec2(1.0, 0.0) == colorRed.xy) && vec3(1.0, 0.0, 0.0) == colorRed.xyz) && vec4(1.0, 0.0, 0.0, 1.0) == colorRed ? colorGreen : colorRed;
}
