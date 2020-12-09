
out vec4 sk_FragColor;
in float a;
void main() {
    sk_FragColor.x = float(isnan(a) ? 1 : 0);
}
