
out vec4 sk_FragColor;
uniform float unknownInput;
vec4 main() {
    vec4 h4 = vec4(unknownInput);
    h4 = vec4(vec2(unknownInput), 0.0, 1.0);
    h4 = vec4(0.0, unknownInput, 1.0, 0.0);
    h4 = vec4(0.0, unknownInput, 0.0, unknownInput);
    return h4;
}
