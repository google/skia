#version 400
out vec4 sk_FragColor;
uniform vec4 src;
uniform vec4 dst;
vec4 blend_screen_h4h4h4(vec4 src, vec4 dst);
vec4 blend_screen_h4h4h4(vec4 src, vec4 dst) {
    return src + (1.0 - src) * dst;
}
void main() {
    sk_FragColor = blend_screen_h4h4h4(src, dst);
}
