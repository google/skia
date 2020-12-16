
out vec4 sk_FragColor;
void main() {
    int i = 0;
    for (; ; ) {
        {
            ++i;
            for (; ; ) {
                {
                    i++;
                }
                if (!true) break;
            }
        }
        if (!(i < 10)) break;
    }
    sk_FragColor = vec4(float(i));
}
