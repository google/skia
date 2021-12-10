cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_testInputs : packoffset(c0);
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
    float4 v = _10_testInputs;
    v = float4(v.x, 1.0f, 1.0f, 1.0f);
    v = float4(v.xy, 1.0f, 1.0f);
    v = float4(v.x, 1.0f, 1.0f, 1.0f);
    v = float4(0.0f, v.y, 1.0f, 1.0f);
    v = float4(v.xyz, 1.0f);
    v = float4(v.xy, 1.0f, 1.0f);
    v = float4(v.x, 0.0f, v.z, 1.0f);
    v = float4(v.x, 1.0f, 0.0f, 1.0f);
    v = float4(1.0f, v.yz, 1.0f);
    v = float4(0.0f, v.y, 1.0f, 1.0f);
    v = float4(1.0f, 1.0f, v.z, 1.0f);
    v = float4(v.xyz, 1.0f);
    v = float4(v.xy, 0.0f, v.w);
    v = float4(v.xy, 1.0f, 0.0f);
    v = float4(v.x, 1.0f, v.zw);
    v = float4(v.x, 0.0f, v.z, 1.0f);
    v = float4(v.x, 1.0f, 1.0f, v.w);
    v = float4(v.x, 1.0f, 0.0f, 1.0f);
    v = float4(1.0f, v.yzw);
    v = float4(0.0f, v.yz, 1.0f);
    v = float4(0.0f, v.y, 1.0f, v.w);
    v = float4(1.0f, v.y, 1.0f, 1.0f);
    v = float4(0.0f, 0.0f, v.zw);
    v = float4(0.0f, 0.0f, v.z, 1.0f);
    v = float4(0.0f, 1.0f, 1.0f, v.w);
    float4 _152 = 0.0f.xxxx;
    if (all(bool4(v.x == float4(0.0f, 1.0f, 1.0f, 1.0f).x, v.y == float4(0.0f, 1.0f, 1.0f, 1.0f).y, v.z == float4(0.0f, 1.0f, 1.0f, 1.0f).z, v.w == float4(0.0f, 1.0f, 1.0f, 1.0f).w)))
    {
        _152 = _10_colorGreen;
    }
    else
    {
        _152 = _10_colorRed;
    }
    return _152;
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
