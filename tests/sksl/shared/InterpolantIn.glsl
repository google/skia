
out vec4 sk_FragColor;
layout (location = 0) in float defaultVarying;
layout (location = 1) noperspective in float linearVarying;
layout (location = 2) flat in float flatVarying;
void main() {
    sk_FragColor = vec4(defaultVarying, linearVarying, flatVarying, 1.0);
}
