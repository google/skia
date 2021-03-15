
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
uniform float unknownInput;
bool test() {
    bool expr = unknownInput > 0.0;
    int ok = 0;
    int bad = 0;
    if (expr) {
        ++ok;
    } else {
        ++bad;
    }
    {
        ++ok;
    }
    if (true ^^ expr) {
        ++bad;
    } else {
        ++ok;
    }
    if (expr) {
        ++ok;
    } else {
        ++bad;
    }
    {
        ++ok;
    }
    if (expr) {
        ++ok;
    } else {
        ++bad;
    }
    if (expr) {
        ++ok;
    } else {
        ++bad;
    }
    if (false == expr) {
        ++bad;
    } else {
        ++ok;
    }
    if (true != expr) {
        ++bad;
    } else {
        ++ok;
    }
    if (expr) {
        ++ok;
    } else {
        ++bad;
    }
    if (expr) {
        ++ok;
    } else {
        ++bad;
    }
    {
        ++ok;
    }
    if (expr ^^ true) {
        ++bad;
    } else {
        ++ok;
    }
    if (expr) {
        ++ok;
    } else {
        ++bad;
    }
    {
        ++ok;
    }
    if (expr) {
        ++ok;
    } else {
        ++bad;
    }
    if (expr) {
        ++ok;
    } else {
        ++bad;
    }
    if (expr == false) {
        ++bad;
    } else {
        ++ok;
    }
    if (expr != true) {
        ++bad;
    } else {
        ++ok;
    }
    if (expr) {
        ++ok;
    } else {
        ++bad;
    }
    float a = sqrt(1.0);
    float b = sqrt(2.0);
    if (a == b) {
        ++bad;
    } else {
        ++ok;
    }
    bool(a = b) || true;
    if (a == b) {
        ++ok;
    } else {
        ++bad;
    }
    return ok == 22 && bad == 0;
}
vec4 main() {
    return test() ? colorGreen : colorRed;
}
