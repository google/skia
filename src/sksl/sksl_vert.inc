STRINGIFY(

// defines built-in interfaces supported by SkiaSL vertex shaders

out sk_PerVertex {
    layout(builtin=0) float4 sk_Position;
    layout(builtin=1) float sk_PointSize;
    layout(builtin=3) float sk_ClipDistance[1];
};

layout(builtin=42) in int sk_VertexID;
layout(builtin=43) in int sk_InstanceID;

)
