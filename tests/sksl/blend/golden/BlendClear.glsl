#version 400
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
vec4 blend_clear(vec4 src, vec4 dst) {
    return vec4(0.0);
}
void main() {
    sk_FragColor = vec4(0.0);

}
