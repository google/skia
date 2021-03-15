/*#pragma settings CannotUseFragCoord*/

layout(location=0) in float4 pos;

void main() {
    sk_Position = pos;
}
