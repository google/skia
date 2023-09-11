cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_testInputs : packoffset(c0);
    float4 _7_colorGreen : packoffset(c1);
    float4 _7_colorRed : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    float4 v = _7_testInputs;
    float4 _32 = float4(_7_testInputs.x, 1.0f, 1.0f, 1.0f);
    v = _32;
    float2 _33 = _32.xy;
    float _34 = _33.x;
    v = float4(_34, _33.y, 1.0f, 1.0f);
    v = float4(_34, 1.0f, 1.0f, 1.0f);
    v = float4(0.0f, 1.0f, 1.0f, 1.0f);
    float4 _44 = float4(float4(0.0f, 1.0f, 1.0f, 1.0f).xyz, 1.0f);
    v = _44;
    float2 _45 = _44.xy;
    float _46 = _45.x;
    v = float4(_46, _45.y, 1.0f, 1.0f);
    v = float4(_46, 0.0f, 1.0f, 1.0f);
    float4 _50 = float4(_46, 1.0f, 0.0f, 1.0f);
    v = _50;
    float2 _51 = _50.yz;
    float _52 = _51.x;
    v = float4(1.0f, _52, _51.y, 1.0f);
    v = float4(0.0f, _52, 1.0f, 1.0f);
    v = 1.0f.xxxx;
    float4 _61 = float4(1.0f.xxxx.xyz, 1.0f);
    v = _61;
    float4 _65 = float4(_61.xy, 0.0f, 1.0f);
    v = _65;
    float2 _66 = _65.xy;
    float _67 = _66.x;
    float4 _69 = float4(_67, _66.y, 1.0f, 0.0f);
    v = _69;
    float2 _70 = _69.zw;
    float _71 = _70.x;
    v = float4(_67, 1.0f, _71, _70.y);
    v = float4(_67, 0.0f, _71, 1.0f);
    v = float4(_67, 1.0f, 1.0f, 1.0f);
    float4 _76 = float4(_67, 1.0f, 0.0f, 1.0f);
    v = _76;
    float4 _81 = float4(1.0f, _76.yzw);
    v = _81;
    float2 _82 = _81.yz;
    float _83 = _82.x;
    v = float4(0.0f, _83, _82.y, 1.0f);
    v = float4(0.0f, _83, 1.0f, 1.0f);
    float4 _87 = float4(1.0f, _83, 1.0f, 1.0f);
    v = _87;
    float2 _88 = _87.zw;
    float _89 = _88.x;
    v = float4(0.0f, 0.0f, _89, _88.y);
    v = float4(0.0f, 0.0f, _89, 1.0f);
    v = float4(0.0f, 1.0f, 1.0f, 1.0f);
    float4 _95 = 0.0f.xxxx;
    if (true)
    {
        _95 = _7_colorGreen;
    }
    else
    {
        _95 = _7_colorRed;
    }
    return _95;
}

void frag_main()
{
    float2 _17 = 0.0f.xx;
    sk_FragColor = main(_17);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
