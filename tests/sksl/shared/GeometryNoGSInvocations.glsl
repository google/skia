#version 400
int sk_InvocationID;
layout (points) in ;
layout (line_strip, max_vertices = 4) out ;
void main() {
    for (sk_InvocationID = 0;sk_InvocationID < 2; sk_InvocationID++) {
        gl_Position = gl_in[0].gl_Position + vec4(0.5, 0.0, 0.0, float(sk_InvocationID));
        EmitVertex();
        gl_Position = gl_in[0].gl_Position + vec4(-0.5, 0.0, 0.0, float(sk_InvocationID));
        EmitVertex();
        false;
        EndPrimitive();
    }
}
