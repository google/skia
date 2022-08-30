
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
bool test_b() {
    bool a = true;
    bool b = false;
    bool c = true;
    bool d = false;
    bool e = true;
    bool f = false;
    bool g = true;
    bool h = false;
    bool i = true;
    bool j = false;
    bool k = true;
    bool l = false;
    bool m = true;
    bool n = false;
    bool o = true;
    bool p = false;
    bool q = true;
    bool r = false;
    bool s = true;
    bool t = false;
    bool u = true;
    bool v = false;
    return ((((((((((((((((((((a && !b) && c) && !d) && e) && !f) && g) && !h) && i) && !j) && k) && !l) && m) && !n) && o) && !p) && q) && !r) && s) && !t) && u) && !v;
}
vec4 main() {
    return test_b() ? colorGreen : colorRed;
}
