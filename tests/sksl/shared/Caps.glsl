
out vec4 sk_FragColor;
void main() {
    int x = 0;
    int y = 0;
    int z = 0;
    x = 1;
    z = 1;
    sk_FragColor.xyz = vec3(float(x), float(y), float(z));
}
