
out vec4 sk_FragColor;
uniform float unknownInput;
vec4 main() {
    float x = unknownInput;
    vec4 v = vec4(vec2(x), float(0), float(1));
    v = vec4(vec2(unknownInput), float(0), float(1));
    v = vec3(unknownInput, float(0), float(1)).yxzy;
    v = vec3(vec2(unknownInput), float(0)).zxzy;
    return v;
}
