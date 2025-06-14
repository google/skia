cbuffer _UniformBuffer : register(b0, space0)
{
    row_major float2x2 _11_testMatrix2x2 : packoffset(c0);
    float4 _11_colorGreen : packoffset(c2);
    float4 _11_colorRed : packoffset(c3);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(out float2 _26)
{
    _26 = sqrt(float4(-1.0f, -4.0f, -16.0f, -64.0f)).xy;
    float4 _50 = float4(_11_testMatrix2x2[0].x, _11_testMatrix2x2[0].y, _11_testMatrix2x2[1].x, _11_testMatrix2x2[1].y) + float4(0.0f, 2.0f, 6.0f, 12.0f);
    float4 inputVal = _50;
    bool _71 = false;
    if (abs(sqrt(_50.x) - 1.0f) < 0.0500000007450580596923828125f)
    {
        float2 _64 = abs(sqrt(_50.xy) - float2(1.0f, 2.0f));
        _71 = all(bool2(_64.x < 0.0500000007450580596923828125f.xx.x, _64.y < 0.0500000007450580596923828125f.xx.y));
    }
    else
    {
        _71 = false;
    }
    bool _85 = false;
    if (_71)
    {
        float3 _76 = abs(sqrt(_50.xyz) - float3(1.0f, 2.0f, 3.0f));
        _85 = all(bool3(_76.x < 0.0500000007450580596923828125f.xxx.x, _76.y < 0.0500000007450580596923828125f.xxx.y, _76.z < 0.0500000007450580596923828125f.xxx.z));
    }
    else
    {
        _85 = false;
    }
    bool _97 = false;
    if (_85)
    {
        float4 _90 = abs(sqrt(_50) - float4(1.0f, 2.0f, 3.0f, 4.0f));
        _97 = all(bool4(_90.x < 0.0500000007450580596923828125f.xxxx.x, _90.y < 0.0500000007450580596923828125f.xxxx.y, _90.z < 0.0500000007450580596923828125f.xxxx.z, _90.w < 0.0500000007450580596923828125f.xxxx.w));
    }
    else
    {
        _97 = false;
    }
    float4 _98 = 0.0f.xxxx;
    if (_97)
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
    float2 _22 = 0.0f.xx;
    float4 _24 = main(_22);
    sk_FragColor = _24;
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
