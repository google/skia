
out vec4 sk_FragColor;
uniform vec4 color;
float count = 0.0;
vec4 trueSide_h4h4(vec4 v) {
    count += 1.0;
    return vec4(sin(v.x), sin(v.y), sin(v.z), sin(v.w));
}
vec4 falseSide_h4h4(vec4 v) {
    count += 1.0;
    return vec4(cos(v.y), cos(v.z), cos(v.w), cos(v.z));
}
void main() {
    sk_FragColor = color.x <= 0.5 ? trueSide_h4h4(color) : falseSide_h4h4(color);
    sk_FragColor *= count;
}
