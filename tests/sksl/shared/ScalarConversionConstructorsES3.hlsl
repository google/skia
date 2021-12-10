cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorGreen : packoffset(c0);
    float4 _10_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    float f = _10_colorGreen.y;
    int i = int(_10_colorGreen.y);
    uint u = uint(_10_colorGreen.y);
    bool b = _10_colorGreen.y != 0.0f;
    float f1 = f;
    float f2 = float(i);
    float f3 = float(u);
    float f4 = float(b);
    int i1 = int(f);
    int i2 = i;
    int i3 = int(u);
    int i4 = int(b);
    uint u1 = uint(f);
    uint u2 = uint(i);
    uint u3 = u;
    uint u4 = uint(b);
    bool b1 = f != 0.0f;
    bool b2 = i != 0;
    bool b3 = u != 0u;
    bool b4 = b;
    float4 _146 = 0.0f.xxxx;
    if ((((((((((((((((f1 + f2) + f3) + f4) + float(i1)) + float(i2)) + float(i3)) + float(i4)) + float(u1)) + float(u2)) + float(u3)) + float(u4)) + float(b1)) + float(b2)) + float(b3)) + float(b4)) == 16.0f)
    {
        _146 = _10_colorGreen;
    }
    else
    {
        _146 = _10_colorRed;
    }
    return _146;
}

void frag_main()
{
    float2 _20 = 0.0f.xx;
    sk_FragColor = main(_20);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
