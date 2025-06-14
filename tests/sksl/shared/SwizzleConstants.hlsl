cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_testInputs : packoffset(c0);
    float4 _11_colorGreen : packoffset(c1);
    float4 _11_colorRed : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    float4 v = _11_testInputs;
    float4 _35 = float4(_11_testInputs.x, 1.0f, 1.0f, 1.0f);
    v = _35;
    float2 _36 = _35.xy;
    float _37 = _36.x;
    v = float4(_37, _36.y, 1.0f, 1.0f);
    v = float4(_37, 1.0f, 1.0f, 1.0f);
    v = float4(0.0f, 1.0f, 1.0f, 1.0f);
    float4 _47 = float4(float4(0.0f, 1.0f, 1.0f, 1.0f).xyz, 1.0f);
    v = _47;
    float2 _48 = _47.xy;
    float _49 = _48.x;
    v = float4(_49, _48.y, 1.0f, 1.0f);
    v = float4(_49, 0.0f, 1.0f, 1.0f);
    float4 _53 = float4(_49, 1.0f, 0.0f, 1.0f);
    v = _53;
    float2 _54 = _53.yz;
    float _55 = _54.x;
    v = float4(1.0f, _55, _54.y, 1.0f);
    v = float4(0.0f, _55, 1.0f, 1.0f);
    v = 1.0f.xxxx;
    float4 _64 = float4(1.0f.xxxx.xyz, 1.0f);
    v = _64;
    float4 _68 = float4(_64.xy, 0.0f, 1.0f);
    v = _68;
    float2 _69 = _68.xy;
    float _70 = _69.x;
    float4 _72 = float4(_70, _69.y, 1.0f, 0.0f);
    v = _72;
    float2 _73 = _72.zw;
    float _74 = _73.x;
    v = float4(_70, 1.0f, _74, _73.y);
    v = float4(_70, 0.0f, _74, 1.0f);
    v = float4(_70, 1.0f, 1.0f, 1.0f);
    float4 _79 = float4(_70, 1.0f, 0.0f, 1.0f);
    v = _79;
    float4 _84 = float4(1.0f, _79.yzw);
    v = _84;
    float2 _85 = _84.yz;
    float _86 = _85.x;
    v = float4(0.0f, _86, _85.y, 1.0f);
    v = float4(0.0f, _86, 1.0f, 1.0f);
    float4 _90 = float4(1.0f, _86, 1.0f, 1.0f);
    v = _90;
    float2 _91 = _90.zw;
    float _92 = _91.x;
    v = float4(0.0f, 0.0f, _92, _91.y);
    v = float4(0.0f, 0.0f, _92, 1.0f);
    v = float4(0.0f, 1.0f, 1.0f, 1.0f);
    float4 _98 = 0.0f.xxxx;
    if (true)
    {
        _98 = _11_colorGreen;
    }
    else
    {
        _98 = _11_colorRed;
    }
    return _98;
}

void frag_main()
{
    float2 _21 = 0.0f.xx;
    sk_FragColor = main(_21);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
