
out vec4 sk_FragColor;
uniform vec4 colorGreen;
uniform vec4 colorRed;
vec4 main() {
    vec3 v2 = vec3(3.0) * mat3(3.0);
    return vec4(float(all(greaterThanEqual(v2, vec3(8.9999904632568359))) && all(lessThanEqual(v2, vec3(9.0000095367431641))) ? 1 : 0), float(all(greaterThanEqual(v2, vec3(8.9998998641967773))) && all(lessThanEqual(v2, vec3(9.0001001358032227))) ? 1 : 0), float(all(greaterThanEqual(v2, vec3(8.9989995956420898))) && all(lessThanEqual(v2, vec3(9.0010004043579102))) ? 1 : 0), float(all(greaterThanEqual(v2, vec3(8.9899997711181641))) && all(lessThanEqual(v2, vec3(9.0100002288818359))) ? 1 : 0));
}
