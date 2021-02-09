#version 400
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
vec4 blend_src(vec4 src, vec4 dst) {
    return src;
}
void main() {
    sk_FragColor = blend_src(src, dst);
}
