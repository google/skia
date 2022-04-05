
out vec4 sk_FragColor;
uniform vec4 colorGreen;
vec4 main() {
    return vec4(float(int(colorGreen.x)), colorGreen.y, colorGreen.zw);
}
