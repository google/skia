#version 400
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool switch_fallthrough_groups_bi(int value) {
    bool ok = false;
    int _tmpSwitchValue1 = value, _tmpSwitchFallthrough0 = 0;
    for (int _tmpSwitchLoop2 = 0; _tmpSwitchLoop2 < 1; _tmpSwitchLoop2++) {
        if ((_tmpSwitchValue1 == -1)) {
            ok = false;
            _tmpSwitchFallthrough0 = 1;
        }
        if ((_tmpSwitchFallthrough0 > 0) || (_tmpSwitchValue1 == 0)) {
            return false;
            _tmpSwitchFallthrough0 = 1;
        }
        if ((_tmpSwitchFallthrough0 > 0) || (_tmpSwitchValue1 == 1)) {
            ok = true;
            _tmpSwitchFallthrough0 = 1;
        }
        if ((_tmpSwitchFallthrough0 > 0) || (_tmpSwitchValue1 == 2)) {
            ;
            _tmpSwitchFallthrough0 = 1;
        }
        if ((_tmpSwitchFallthrough0 > 0) || (_tmpSwitchValue1 == 3)) {
            break;
            _tmpSwitchFallthrough0 = 1;
        }
        if ((_tmpSwitchFallthrough0 > 0) || (_tmpSwitchValue1 == 4)) {
            ok = false;
            _tmpSwitchFallthrough0 = 1;
        }
        if ((_tmpSwitchFallthrough0 > 0) || (_tmpSwitchValue1 == 5)) {
            ;
            _tmpSwitchFallthrough0 = 1;
        }
        if ((_tmpSwitchFallthrough0 > 0) || (_tmpSwitchValue1 == 6)) {
            ;
            _tmpSwitchFallthrough0 = 1;
        }
        if ((_tmpSwitchFallthrough0 > 0) || (_tmpSwitchValue1 == 7)) {
            ;
            _tmpSwitchFallthrough0 = 1;
        }
        break;
    }
    return ok;
}
vec4 main() {
    int x = int(colorGreen.y);
    return switch_fallthrough_groups_bi(x) ? colorGreen : colorRed;
}
