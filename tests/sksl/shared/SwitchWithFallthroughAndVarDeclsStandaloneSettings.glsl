
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    bool ok;
    {
        bool a;
        const int ONE = 1;
        int b;
        int c;
        switch (int(colorGreen.y)) {
            case 0:
            case 1:
            case 2:
                b = ONE;
            case 3:
                {
                    float d = float(b);
                    c = int(d);
                }
            case 4:
                a = bool(c);
            case 5:
                ok = a;
        }
    }
    return ok ? colorGreen : colorRed;
}
