
out vec4 sk_FragColor;
void main() {
    (sk_FragColor.x = dFdx(1.0) , sk_FragColor.y = dFdy(1.0));
}
