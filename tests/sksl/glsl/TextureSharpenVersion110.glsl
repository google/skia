#version 110
uniform sampler2D s;
void main() {
    vec4 a = texture2D(s, vec2(0.0), -0.475);
    vec4 b = texture2DProj(s, vec3(0.0), -0.475);
    gl_FragColor = vec4(a.xy, b.xy);
}
