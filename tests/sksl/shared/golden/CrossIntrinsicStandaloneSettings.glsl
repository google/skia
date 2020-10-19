
out vec4 sk_FragColor;
uniform vec2 h2;
uniform vec2 f2;
void main() {
    vec2 _1_a = vec2(1.0, 2.0);
    vec2 _2_b = vec2(3.0, 4.0);

    sk_FragColor = vec4(-2.0);

    vec2 _4_a = vec2(5.0, 6.0);
    vec2 _5_b = vec2(7.0, 8.0);

    sk_FragColor = vec4(-2.0);

    float _6_cross;
    {
        _6_cross = h2.x * h2.y - h2.y * h2.x;
    }

    sk_FragColor = vec4(_6_cross);

    float _7_cross;
    {
        _7_cross = f2.x * f2.y - f2.y * f2.x;
    }

    sk_FragColor = vec4(_7_cross);

}
