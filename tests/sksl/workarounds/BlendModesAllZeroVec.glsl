
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
void main() {
    sk_FragColor = src == vec4(0.0) ? vec4(0.0) : src * dst.w;
    sk_FragColor = dst == vec4(0.0) ? vec4(0.0) : dst * src.w;
}
