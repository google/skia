
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform float unknownInput;
vec4 main() {
    vec4 result;
    result.x = colorGreen.x;
    result.y = colorGreen.y;
    result.z = colorGreen.z;
    result.w = colorGreen.w;
    return result;
}
