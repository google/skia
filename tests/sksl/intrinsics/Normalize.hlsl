cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_inputVal : packoffset(c0);
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
    float4 expectedVec = float4(1.0f, 0.0f, 0.0f, 0.0f);
    bool _52 = false;
    if (sign(_10_inputVal.x) == expectedVec.x)
    {
        float2 _43 = normalize(_10_inputVal.xy);
        _52 = all(bool2(_43.x == expectedVec.xy.x, _43.y == expectedVec.xy.y));
    }
    else
    {
        _52 = false;
    }
    bool _65 = false;
    if (_52)
    {
        float3 _55 = normalize(_10_inputVal.xyz);
        _65 = all(bool3(_55.x == expectedVec.xyz.x, _55.y == expectedVec.xyz.y, _55.z == expectedVec.xyz.z));
    }
    else
    {
        _65 = false;
    }
    bool _75 = false;
    if (_65)
    {
        float4 _68 = normalize(_10_inputVal);
        _75 = all(bool4(_68.x == expectedVec.x, _68.y == expectedVec.y, _68.z == expectedVec.z, _68.w == expectedVec.w));
    }
    else
    {
        _75 = false;
    }
    bool _81 = false;
    if (_75)
    {
        _81 = 1.0f == expectedVec.x;
    }
    else
    {
        _81 = false;
    }
    bool _89 = false;
    if (_81)
    {
        _89 = all(bool2(float2(0.0f, 1.0f).x == expectedVec.yx.x, float2(0.0f, 1.0f).y == expectedVec.yx.y));
    }
    else
    {
        _89 = false;
    }
    bool _97 = false;
    if (_89)
    {
        _97 = all(bool3(float3(0.0f, 1.0f, 0.0f).x == expectedVec.zxy.x, float3(0.0f, 1.0f, 0.0f).y == expectedVec.zxy.y, float3(0.0f, 1.0f, 0.0f).z == expectedVec.zxy.z));
    }
    else
    {
        _97 = false;
    }
    bool _103 = false;
    if (_97)
    {
        _103 = all(bool4(float4(1.0f, 0.0f, 0.0f, 0.0f).x == expectedVec.x, float4(1.0f, 0.0f, 0.0f, 0.0f).y == expectedVec.y, float4(1.0f, 0.0f, 0.0f, 0.0f).z == expectedVec.z, float4(1.0f, 0.0f, 0.0f, 0.0f).w == expectedVec.w));
    }
    else
    {
        _103 = false;
    }
    float4 _104 = 0.0f.xxxx;
    if (_103)
    {
        _104 = _10_colorGreen;
    }
    else
    {
        _104 = _10_colorRed;
    }
    return _104;
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
