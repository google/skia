
out vec4 sk_FragColor;
uniform vec4 inColor;
void main() {
    vec4 color = inColor;
    sk_FragColor = color.yzyx;
    vec4 _0_flip;
    vec4 _1_v = color.xyzy;
    {
        _0_flip = _1_v.wzyx;
    }

    sk_FragColor = _0_flip;

    {
        color = color.wzyx;
    }


    sk_FragColor = color;
}
