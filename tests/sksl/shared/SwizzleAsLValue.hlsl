cbuffer _UniformBuffer : register(b0, space0)
{
    float _10_testInput : packoffset(c0);
    float4 _10_colorGreen : packoffset(c1);
    float4 _10_colorRed : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    float4 color = _10_colorGreen * 0.5f;
    color.w = 2.0f;
    color.y /= 0.25f;
    float3 _47 = color.yzw * 0.5f.xxx;
    color = float4(color.x, _47.x, _47.y, _47.z);
    float4 _50 = color;
    float4 _54 = _50.zywx + float4(0.25f, 0.0f, 0.0f, 0.75f);
    float4 _55 = color;
    float4 _56 = float4(_54.w, _54.y, _54.x, _54.z);
    color = _56;
    float4 _62 = 0.0f.xxxx;
    if (all(bool4(_56.x == float4(0.75f, 1.0f, 0.25f, 1.0f).x, _56.y == float4(0.75f, 1.0f, 0.25f, 1.0f).y, _56.z == float4(0.75f, 1.0f, 0.25f, 1.0f).z, _56.w == float4(0.75f, 1.0f, 0.25f, 1.0f).w)))
    {
        _62 = _10_colorGreen;
    }
    else
    {
        _62 = _10_colorRed;
    }
    return _62;
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
