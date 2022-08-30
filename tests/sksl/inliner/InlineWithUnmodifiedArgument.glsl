
out vec4 sk_FragColor;
float square_hh(float x) {
    return x * x;
}
vec4 main() {
    float one = 1.0;
    vec4 result = vec4(123.0);
    result.x = square_hh(0.0);
    result.z = square_hh(result.x);
    result.y = square_hh(1.0);
    result.w = square_hh(one);
    return result;
}
