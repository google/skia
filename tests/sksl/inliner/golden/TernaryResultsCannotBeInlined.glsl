
precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
uniform mediump vec4 color;
mediump float count = 0.0;
mediump vec4 trueSide(mediump vec4 v) {
    count += 1.0;
    return vec4(sin(v.x), sin(v.y), sin(v.z), sin(v.w));
}
mediump vec4 falseSide(mediump vec4 v) {
    count += 1.0;
    return vec4(cos(v.y), cos(v.z), cos(v.w), cos(v.z));
}
void main() {
    sk_FragColor = color.x <= 0.5 ? trueSide(color) : falseSide(color);
    sk_FragColor *= count;
}
