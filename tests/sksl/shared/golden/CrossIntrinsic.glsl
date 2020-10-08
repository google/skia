#version 400
out vec4 sk_FragColor;
uniform vec2 h2;
uniform vec2 f2;
void main() {
    float _0_cross;
    {
        _0_cross = determinant(mat2(vec2(1.0, 2.0), vec2(3.0, 4.0)));
    }

    sk_FragColor = vec4(_0_cross);

    float _3_cross;
    {
        _3_cross = determinant(mat2(vec2(5.0, 6.0), vec2(7.0, 8.0)));
    }

    sk_FragColor = vec4(_3_cross);

    float _6_cross;
    {
        _6_cross = determinant(mat2(h2, h2));
    }

    sk_FragColor = vec4(_6_cross);

    float _7_cross;
    {
        _7_cross = determinant(mat2(f2, f2));
    }

    sk_FragColor = vec4(_7_cross);

}
