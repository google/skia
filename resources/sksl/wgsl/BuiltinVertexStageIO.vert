void main() {
    // Declarations for all fragment function built-ins referenced here should be present
    // in the output.
    float x = float(sk_VertexID);
    float y = float(sk_InstanceID);

    sk_PointSize = x;
    sk_Position = float4(x, y, 1, 1);
}
