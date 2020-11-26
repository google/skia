
out vec4 sk_FragColor;
void main() {
    {
        sk_FragColor = vec4(1.0);
    }
    {
        sk_FragColor = vec4(2.0);
    }
    {
        sk_FragColor = vec4(6.0);
    }
    sk_FragColor = vec4(7.0);
    sk_FragColor = vec4(-8.0);
    sk_FragColor = vec4(-9.0);
    sk_FragColor = vec4(10.0);
    {
        sk_FragColor = vec4(11.0);
    }
    {
        sk_FragColor = vec4(13.0);
    }
}
