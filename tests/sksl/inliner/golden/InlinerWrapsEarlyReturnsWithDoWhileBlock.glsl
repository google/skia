#version 400
out vec4 sk_FragColor;
uniform vec4 color;
void main() {
    vec4 _0_returny;
    do {
        if (color.x > color.y) {
            _0_returny = color.xxxx;
            break;
        }
        if (color.y > color.z) {
            _0_returny = color.yyyy;
            break;
        }
        {
            _0_returny = color.zzzz;
            break;
        }
    } while (false);
    sk_FragColor = _0_returny;

}
