
out vec4 sk_FragColor;
void main() {
    vec2 x = vec2(1.0);
    x = x;
    x = -x;
    sk_FragColor.xy = x;
}
