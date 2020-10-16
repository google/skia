#version 400
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
vec4 blend_plus(vec4 src, vec4 dst) {
    return min(src + dst, 1.0);
}
void main() {
    sk_FragColor = blend_plus(src, dst);
}
