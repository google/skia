
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    float _0_branchy;
    if (colorGreen.y == 0.0) _0_branchy = 0.0; else if (colorRed.x == 0.0) _0_branchy = 0.0; else if (colorGreen == colorRed) _0_branchy = 0.0; else _0_branchy = 1.0;
    float _1_branchyAndBlocky;
    if (colorGreen.y == 0.0) {
        {
            {
                _1_branchyAndBlocky = 0.0;
            }
        }
    } else if (colorRed.x == 0.0) {
        {
            _1_branchyAndBlocky = 0.0;
        }
    } else {
        if (colorGreen == colorRed) _1_branchyAndBlocky = 0.0; else {
            {
                _1_branchyAndBlocky = 1.0;
            }
        }
    }
    return bool(_0_branchy * _1_branchyAndBlocky) ? colorGreen : colorRed;
}
