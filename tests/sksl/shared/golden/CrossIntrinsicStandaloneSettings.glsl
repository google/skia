
out vec4 sk_FragColor;
uniform vec2 h2;
uniform vec2 f2;
void main() {
    sk_FragColor = vec4(-2.0);

    sk_FragColor = vec4(-2.0);

    float _2_cross;
    {
        _2_cross = h2.x * h2.y - h2.y * h2.x;
    }
    sk_FragColor = vec4(_2_cross);

    float _3_cross;
    {
        _3_cross = f2.x * f2.y - f2.y * f2.x;
    }
    sk_FragColor = vec4(_3_cross);

}
