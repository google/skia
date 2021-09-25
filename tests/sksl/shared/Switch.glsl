#version 400
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec4 color;
    int _tmpSwitchValue1 = int(colorGreen.y), _tmpSwitchFallthrough0 = 0;
    for (int _tmpSwitchLoop2 = 0; _tmpSwitchLoop2 < 1; _tmpSwitchLoop2++) {
        if ((_tmpSwitchValue1 == 0)) {
            color = colorRed;
            break;
            _tmpSwitchFallthrough0 = 1;
        }
        if ((_tmpSwitchFallthrough0 > 0) || (_tmpSwitchValue1 == 1)) {
            color = colorGreen;
            break;
            _tmpSwitchFallthrough0 = 1;
        }
        color = colorRed;
        break;
    }
    return color;
}
