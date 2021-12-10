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
    bool b = _10_colorGreen.y != 0.0f;
    float f1 = f;
    float f2 = float(i);
    float f3 = float(b);
    int i1 = int(f);
    int i2 = i;
    int i3 = int(b);
    bool b1 = f != 0.0f;
    bool b2 = i != 0;
    bool b3 = b;
    float4 _97 = 0.0f.xxxx;
    if (((((((((f1 + f2) + f3) + float(i1)) + float(i2)) + float(i3)) + float(b1)) + float(b2)) + float(b3)) == 9.0f)
    {
        _97 = _10_colorGreen;
    }
    else
    {
        _97 = _10_colorRed;
    }
    return _97;
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
