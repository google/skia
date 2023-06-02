cbuffer _UniformBuffer : register(b0, space0)
{
    row_major float2x2 _10_testMatrix2x2 : packoffset(c0);
    float4 _10_colorGreen : packoffset(c2);
    float4 _10_colorRed : packoffset(c3);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(out float2 _25)
{
    _25 = sqrt(float4(-1.0f, -4.0f, -16.0f, -64.0f)).xy;
    float4 _50 = float4(_10_testMatrix2x2[0].x, _10_testMatrix2x2[0].y, _10_testMatrix2x2[1].x, _10_testMatrix2x2[1].y) + float4(0.0f, 2.0f, 6.0f, 12.0f);
    float4 inputVal = _50;
    bool _70 = false;
    if (abs(sqrt(_50.x) - 1.0f) < 0.0500000007450580596923828125f)
    {
        float2 _63 = abs(sqrt(_50.xy) - float2(1.0f, 2.0f));
        _70 = all(bool2(_63.x < 0.0500000007450580596923828125f.xx.x, _63.y < 0.0500000007450580596923828125f.xx.y));
    }
    else
    {
        _70 = false;
    }
    bool _84 = false;
    if (_70)
    {
        float3 _75 = abs(sqrt(_50.xyz) - float3(1.0f, 2.0f, 3.0f));
        _84 = all(bool3(_75.x < 0.0500000007450580596923828125f.xxx.x, _75.y < 0.0500000007450580596923828125f.xxx.y, _75.z < 0.0500000007450580596923828125f.xxx.z));
    }
    else
    {
        _84 = false;
    }
    bool _96 = false;
    if (_84)
    {
        float4 _89 = abs(sqrt(_50) - float4(1.0f, 2.0f, 3.0f, 4.0f));
        _96 = all(bool4(_89.x < 0.0500000007450580596923828125f.xxxx.x, _89.y < 0.0500000007450580596923828125f.xxxx.y, _89.z < 0.0500000007450580596923828125f.xxxx.z, _89.w < 0.0500000007450580596923828125f.xxxx.w));
    }
    else
    {
        _96 = false;
    }
    float4 _97 = 0.0f.xxxx;
    if (_96)
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
    float2 _21 = 0.0f.xx;
    float4 _23 = main(_21);
    sk_FragColor = _23;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
