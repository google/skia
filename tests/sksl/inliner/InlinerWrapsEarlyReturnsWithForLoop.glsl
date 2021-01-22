#version 400
out vec4 sk_FragColor;
uniform vec4 color;
void main() {
    vec4 _0_returny;
    for (int _1_loop = 0;_1_loop < 1; _1_loop++) {
        if (color.x > color.y) {
            _0_returny = color.xxxx;
            continue;
        }
        if (color.y > color.z) {
            _0_returny = color.yyyy;
            continue;
        }
        {
            _0_returny = color.zzzz;
            continue;
        }
    }
    sk_FragColor = _0_returny;

}
