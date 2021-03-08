#version 400
out vec4 sk_FragColor;
void main() {
    float minAbsHackVar0;
    float minAbsHackVar1;
    float x = -5.0;
    sk_FragColor.x = ((minAbsHackVar0 = abs(x)) < (minAbsHackVar1 = 6.0) ? minAbsHackVar0 : minAbsHackVar1);
}
