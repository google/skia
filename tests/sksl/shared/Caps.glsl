
out vec4 sk_FragColor;
void main() {
    int x = 0;
    int y = 0;
    int z = 0;
    if (true) x = 1;
    if (false) y = 1;
    if (true) z = 1;
    sk_FragColor.xyz = vec3(float(x), float(y), float(z));
}
