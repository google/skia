
out vec4 sk_FragColor;
void main() {
    sk_FragColor.x = float((1.0 , 1));
    sk_FragColor.y = float((vec2(1.0) , 1));
    sk_FragColor.z = float((vec3(1.0) , 1));
    sk_FragColor.w = float((mat2(1.0) , 1));
}
