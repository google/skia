#version 400
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
vec4 blend_dst(vec4 src, vec4 dst) {
    return dst;
}
void main() {
    sk_FragColor = blend_dst(src, dst);
}
