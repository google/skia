#version 400
out vec4 sk_FragColor;
uniform vec2 h2;
uniform vec2 f2;
void main() {
    float _0_cross;
    {
        _0_cross = determinant(mat2(1.0, 2.0, 3.0, 4.0));
    }
    sk_FragColor = vec4(_0_cross);

    float _1_cross;
    {
        _1_cross = determinant(mat2(5.0, 6.0, 7.0, 8.0));
    }
    sk_FragColor = vec4(_1_cross);

    float _2_cross;
    {
        _2_cross = determinant(mat2(h2, h2));
    }
    sk_FragColor = vec4(_2_cross);

    float _3_cross;
    {
        _3_cross = determinant(mat2(f2, f2));
    }
    sk_FragColor = vec4(_3_cross);

}
