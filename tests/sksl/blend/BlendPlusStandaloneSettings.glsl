
out vec4 sk_FragColor;
uniform vec4 src;
uniform vec4 dst;
vec4 blend_plus_h4h4h4(vec4 src, vec4 dst);
vec4 blend_plus_h4h4h4(vec4 src, vec4 dst) {
    return min(src + dst, 1.0);
}
void main() {
    sk_FragColor = blend_plus_h4h4h4(src, dst);
}
