
out vec4 sk_FragColor;
uniform vec4 src;
uniform vec4 dst;
void main() {
    vec4 _0_blend;
    for (int _1_loop = 0;_1_loop < 1; _1_loop++) {
        {
            {
                _0_blend = src * dst;
                continue;
            }
        }
    }
    sk_FragColor = _0_blend;
}
