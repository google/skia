
out vec4 sk_FragColor;
uniform float myHalf;
uniform vec4 myHalf4;
vec4 main() {
    return myHalf4 * myHalf;
}
