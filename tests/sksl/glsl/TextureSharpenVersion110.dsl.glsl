#version 110
uniform sampler1D one;
uniform sampler2D two;
void main() {
    vec4 a = texture1D(one, 0.0, -0.5);
    vec4 b = texture2D(two, vec2(0.0), -0.5);
    vec4 c = texture1DProj(one, vec2(0.0), -0.5);
    vec4 d = texture2DProj(two, vec3(0.0), -0.5);
    gl_FragColor = vec4(a.x, b.x, c.x, d.x);
}
