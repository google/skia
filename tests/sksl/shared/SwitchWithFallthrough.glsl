#version 400
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
bool switch_fallthrough_twice_bi(int value) {
    bool ok = false;
    int _tmpSwitchValue1 = value, _tmpSwitchFallthrough0 = 0;
    for (int _tmpSwitchLoop2 = 0; _tmpSwitchLoop2 < 1; _tmpSwitchLoop2++) {
        if ((_tmpSwitchValue1 == 0)) {
            break;
            _tmpSwitchFallthrough0 = 1;
        }
        if ((_tmpSwitchFallthrough0 > 0) || (_tmpSwitchValue1 == 1)) {
            ;
            _tmpSwitchFallthrough0 = 1;
        }
        if ((_tmpSwitchFallthrough0 > 0) || (_tmpSwitchValue1 == 2)) {
            ;
            _tmpSwitchFallthrough0 = 1;
        }
        if ((_tmpSwitchFallthrough0 > 0) || (_tmpSwitchValue1 == 3)) {
            ok = true;
            break;
            _tmpSwitchFallthrough0 = 1;
        }
        break;
    }
    return ok;
}
bool switch_fallthrough_groups_bi(int value) {
    bool ok = false;
    int _tmpSwitchValue4 = value, _tmpSwitchFallthrough3 = 0;
    for (int _tmpSwitchLoop5 = 0; _tmpSwitchLoop5 < 1; _tmpSwitchLoop5++) {
        if ((_tmpSwitchValue4 == -1)) {
            ok = false;
            _tmpSwitchFallthrough3 = 1;
        }
        if ((_tmpSwitchFallthrough3 > 0) || (_tmpSwitchValue4 == 0)) {
            return false;
            _tmpSwitchFallthrough3 = 1;
        }
        if ((_tmpSwitchFallthrough3 > 0) || (_tmpSwitchValue4 == 1)) {
            ok = true;
            _tmpSwitchFallthrough3 = 1;
        }
        if ((_tmpSwitchFallthrough3 > 0) || (_tmpSwitchValue4 == 2)) {
            ;
            _tmpSwitchFallthrough3 = 1;
        }
        if ((_tmpSwitchFallthrough3 > 0) || (_tmpSwitchValue4 == 3)) {
            break;
            _tmpSwitchFallthrough3 = 1;
        }
        if ((_tmpSwitchFallthrough3 > 0) || (_tmpSwitchValue4 == 4)) {
            ok = false;
            _tmpSwitchFallthrough3 = 1;
        }
        if ((_tmpSwitchFallthrough3 > 0) || (_tmpSwitchValue4 == 5)) {
            ;
            _tmpSwitchFallthrough3 = 1;
        }
        if ((_tmpSwitchFallthrough3 > 0) || (_tmpSwitchValue4 == 6)) {
            ;
            _tmpSwitchFallthrough3 = 1;
        }
        if ((_tmpSwitchFallthrough3 > 0) || (_tmpSwitchValue4 == 7)) {
            ;
            _tmpSwitchFallthrough3 = 1;
        }
        break;
    }
    return ok;
}
vec4 main() {
    int x = int(colorGreen.y);
    bool _0_ok = false;
    int _tmpSwitchValue7 = x, _tmpSwitchFallthrough6 = 0;
    for (int _tmpSwitchLoop8 = 0; _tmpSwitchLoop8 < 1; _tmpSwitchLoop8++) {
        if ((_tmpSwitchValue7 == 2)) {
            break;
            _tmpSwitchFallthrough6 = 1;
        }
        if ((_tmpSwitchFallthrough6 > 0) || (_tmpSwitchValue7 == 1)) {
            ;
            _tmpSwitchFallthrough6 = 1;
        }
        if ((_tmpSwitchFallthrough6 > 0) || (_tmpSwitchValue7 == 0)) {
            _0_ok = true;
            break;
            _tmpSwitchFallthrough6 = 1;
        }
        break;
    }
    return (_0_ok && switch_fallthrough_twice_bi(x)) && switch_fallthrough_groups_bi(x) ? colorGreen : colorRed;
}
