
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    bool ok = true;
    ok = ok && vec4(ivec4(0, 0, 1, 2)) == vec4(ivec4(vec4(0.0099999997764825821, 0.99000000953674316, 1.4900000095367432, 2.75)));
    ok = ok && vec4(ivec4(0, 0, -1, -2)) == vec4(ivec4(vec4(-0.0099999997764825821, -0.99000000953674316, -1.4900000095367432, -2.75)));
    return ok ? colorGreen : colorRed;
}
