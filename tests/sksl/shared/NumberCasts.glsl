
out vec4 sk_FragColor;
vec4 main() {
    bvec3 B;
    B.x = true;
    B.y = true;
    B.z = true;
    vec3 F;
    F.x = 1.2300000190734863;
    F.y = 0.0;
    F.z = 1.0;
    ivec3 I;
    I.x = 1;
    I.y = 1;
    I.z = 1;
    return vec4((F.x * F.y) * F.z, float((B.x && B.y) && B.z), 0.0, float((I.x * I.y) * I.z));
}
