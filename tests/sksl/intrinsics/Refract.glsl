
out vec4 sk_FragColor;
uniform float a;
uniform float b;
uniform float c;
uniform vec4 d;
uniform vec4 e;
vec4 main() {
    vec4 result = vec4(refract(6e+26, 2.0, 2.0));
    result.x = refract(a, b, c);
    result = refract(d, e, c);
    result.xy = vec2(0.5, -0.8660254);
    result.xyz = vec3(0.5, 0.0, -0.8660254);
    result = vec4(0.5, 0.0, 0.0, -0.8660254);
    return result;
}
