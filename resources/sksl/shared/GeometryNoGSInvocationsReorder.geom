/*#pragma settings NoGSInvocationsSupport*/

layout(points) in;

// Subtle error: Declaring max_vertices before invocations causes us not to
// apply the workaround fixup to max_vertices. It *should* be 4 (2*2) in the
// GLSL, but is currently only 2. (skbug.com/10827)
layout(line_strip, max_vertices = 2) out;
layout(invocations = 2) in;

void test() {
    sk_Position = sk_in[0].sk_Position + float4(0.5, 0, 0, sk_InvocationID);
    EmitVertex();
}

void main() {
    test();
    sk_Position = sk_in[0].sk_Position + float4(-0.5, 0, 0, sk_InvocationID);
    EmitVertex();
}
