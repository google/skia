#version 400
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
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
