
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
    switch (0) {
        case 0:
            sk_FragColor = vec4(11.0);
            break;
        case 1:
            sk_FragColor = vec4(12.0);
            break;
    }
    {
        sk_FragColor = vec4(13.0);
    }
    {
        sk_FragColor = vec4(15.0);
    }
    {
        sk_FragColor = vec4(16.0);
    }
    {
        sk_FragColor = vec4(18.0);
    }
    sk_FragColor = vec4(19.0);
    sk_FragColor = vec4(20.0);
    {
        sk_FragColor = vec4(21.0);
    }
}
