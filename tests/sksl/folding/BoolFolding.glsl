
out vec4 sk_FragColor;
uniform vec4 colorRed;
uniform vec4 colorGreen;
bool test() {
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
    return ((((((((a && !b) && c) && !d) && e) && !f) && g) && !h) && i) && !j;
}
vec4 main() {
    return test() ? colorGreen : colorRed;
}
