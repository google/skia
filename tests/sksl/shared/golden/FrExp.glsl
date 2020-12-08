
out vec4 sk_FragColor;
void main() {
    int exp1;
    float a = frexp(0.5, exp1);
    sk_FragColor = vec4(float(exp1));
    ivec3 exp3;
    sk_FragColor.xyz = frexp(vec3(3.5), exp3);
}
