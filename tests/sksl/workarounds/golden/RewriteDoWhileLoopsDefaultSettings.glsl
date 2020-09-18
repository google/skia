
out vec4 sk_FragColor;
void main() {
    int i = 0;
    do {
        ++i;
        do {
            i++;
        } while (true);
    } while (i < 10);
    sk_FragColor = vec4(float(i));
}
