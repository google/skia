
out vec4 sk_FragColor;
vec4 main() {
    float one = 1.0;
    vec4 result = vec4(123.0);
    result.x = 0.0;
    result.z = result.x * result.x;
    result.y = 1.0;
    result.w = one * one;
    return result;
}
