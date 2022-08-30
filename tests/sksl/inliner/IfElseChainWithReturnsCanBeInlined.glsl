
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
float branchy_h() {
    if (colorGreen.y == 0.0) return 0.0; else if (colorRed.x == 0.0) return 0.0; else if (colorGreen == colorRed) return 0.0; else return 1.0;
}
float branchyAndBlocky_h() {
    if (colorGreen.y == 0.0) {
        {
            {
                return 0.0;
            }
        }
    } else if (colorRed.x == 0.0) {
        {
            return 0.0;
        }
    } else {
        if (colorGreen == colorRed) return 0.0; else {
            {
                return 1.0;
            }
        }
    }
}
vec4 main() {
    return bool(branchy_h() * branchyAndBlocky_h()) ? colorGreen : colorRed;
}
