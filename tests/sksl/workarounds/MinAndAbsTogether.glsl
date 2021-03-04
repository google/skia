#version 400
out vec4 sk_FragColor;
void main() {
    float minAbsHackVar0;
    float minAbsHackVar1;
    sk_FragColor.x = ((minAbsHackVar0 = abs(-5.0)) < (minAbsHackVar1 = 6.0) ? minAbsHackVar0 : minAbsHackVar1);
}
