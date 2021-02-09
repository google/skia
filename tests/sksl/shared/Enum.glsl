
out vec4 sk_FragColor;
void main() {
    int e = 0;
    if (e == 0) {
        sk_FragColor = vec4(1.0);
    }
    if (e == 0) {
        sk_FragColor = vec4(2.0);
    }
    if (e != 0) {
        sk_FragColor = vec4(3.0);
    }
    if (e == 1) {
        sk_FragColor = vec4(4.0);
    }
    if (e == 1) {
        sk_FragColor = vec4(5.0);
    }
    if (e != 1) {
        sk_FragColor = vec4(6.0);
    }
    sk_FragColor = e == 0 ? vec4(7.0) : vec4(-7.0);
    sk_FragColor = e != 0 ? vec4(8.0) : vec4(-8.0);
    sk_FragColor = e == 1 ? vec4(9.0) : vec4(-9.0);
    sk_FragColor = e != 1 ? vec4(10.0) : vec4(-10.0);
    switch (e) {
        case 0:
            sk_FragColor = vec4(11.0);
            break;
        case 1:
            sk_FragColor = vec4(12.0);
            break;
    }
    switch (e) {
        case 0:
            sk_FragColor = vec4(13.0);
            break;
        case 1:
            sk_FragColor = vec4(14.0);
            break;
    }
    int m = 0;
    if (m == 0) {
        sk_FragColor = vec4(15.0);
    }
    if (m == 0) {
        sk_FragColor = vec4(16.0);
    }
    if (m == 1) {
        sk_FragColor = vec4(17.0);
    }
    if (m != 2) {
        sk_FragColor = vec4(18.0);
    }
    sk_FragColor = m == 0 ? vec4(19.0) : vec4(-19.0);
    sk_FragColor = m != 1 ? vec4(20.0) : vec4(-20.0);
    switch (m) {
        case 0:
            sk_FragColor = vec4(21.0);
            break;
        case 1:
            sk_FragColor = vec4(22.0);
            break;
        case 2:
            sk_FragColor = vec4(23.0);
            break;
    }
}
