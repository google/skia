
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
    int f = 1;
    if (f == 1) {
        sk_FragColor = vec4(1.0);
    }
    if (f != 1) {
        sk_FragColor = vec4(4.0);
    }
    sk_FragColor = f == 0 ? vec4(7.0) : vec4(-7.0);
    sk_FragColor = f != 0 ? vec4(8.0) : vec4(-8.0);
    sk_FragColor = f == 1 ? vec4(9.0) : vec4(-9.0);
    sk_FragColor = f != 1 ? vec4(10.0) : vec4(-10.0);
    switch (f) {
        case 0:
            sk_FragColor = vec4(11.0);
            break;
        case 1:
            sk_FragColor = vec4(12.0);
            break;
    }
}
