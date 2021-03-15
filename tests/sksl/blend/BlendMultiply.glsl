#version 400
out vec4 sk_FragColor;
uniform vec4 src;
uniform vec4 dst;
void main() {
    sk_FragColor = vec4(((1.0 - src.w) * dst.xyz + (1.0 - dst.w) * src.xyz) + src.xyz * dst.xyz, src.w + (1.0 - src.w) * dst.w);
}
