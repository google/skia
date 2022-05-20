#version 400
out vec4 sk_FragColor;
uniform int i;
uniform int j;
void main() {
    bool x = bool(i);
    bool y = bool(j);
    bool andXY = x ? y : false;
    bool orXY = x ? true : y;
    bool combo = (x ? y : false) ? true : (x ? true : y);
    bool prec = i + j == 3 ? y : false;
    while (((andXY ? orXY : false) ? combo : false) ? prec : false) {
        sk_FragColor = vec4(0.0);
        break;
    }
}
