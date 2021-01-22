
out vec4 sk_FragColor;
uniform bool x;
uniform bool y;
uniform int i;
uniform int j;
void main() {
    bool andXY = x && y;
    bool orXY = x || y;
    bool combo = x && y || (x || y);
    bool prec = i + j == 3 && y;
    while (((andXY && orXY) && combo) && prec) {
        sk_FragColor = vec4(0.0);
        break;
    }
}
