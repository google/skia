
out vec4 sk_FragColor;
vec4 blend_dst(vec4 src, vec4 dst) {
    return dst;
}
vec4 live_fn(vec4 a, vec4 b) {
    return a + b;
}
void main() {
    {
        sk_FragColor = live_fn(vec4(3.0), vec4(-3.0));
    }
    {
        sk_FragColor = blend_dst(vec4(7.0), vec4(-7.0));
    }
}
