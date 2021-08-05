
out vec4 sk_FragColor;
vec4 main() {
    mat2 x = mat2(0.0, 1.0, 2.0, 3.0);
    vec2 y = vec4(x).xy;
    return y.xyxy;
}
