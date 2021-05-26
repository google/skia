
out vec4 sk_FragColor;
vec4 main() {
    float huge = inf.0;
    return clamp(vec4(0.0, huge, 0.0, huge), 0.0, 1.0);
}
