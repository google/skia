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
    float4 expected = float4(-2.0f, 0.0f, 0.0f, 2.0f);
    bool _51 = false;
    if (floor(_11_testInputs.x) == (-2.0f))
    {
        float2 _43 = floor(_11_testInputs.xy);
        _51 = all(bool2(_43.x == float4(-2.0f, 0.0f, 0.0f, 2.0f).xy.x, _43.y == float4(-2.0f, 0.0f, 0.0f, 2.0f).xy.y));
    }
    else
    {
        _51 = false;
    }
    bool _63 = false;
    if (_51)
    {
        float3 _54 = floor(_11_testInputs.xyz);
        _63 = all(bool3(_54.x == float4(-2.0f, 0.0f, 0.0f, 2.0f).xyz.x, _54.y == float4(-2.0f, 0.0f, 0.0f, 2.0f).xyz.y, _54.z == float4(-2.0f, 0.0f, 0.0f, 2.0f).xyz.z));
    }
    else
    {
        _63 = false;
    }
    bool _72 = false;
    if (_63)
    {
        float4 _66 = floor(_11_testInputs);
        _72 = all(bool4(_66.x == float4(-2.0f, 0.0f, 0.0f, 2.0f).x, _66.y == float4(-2.0f, 0.0f, 0.0f, 2.0f).y, _66.z == float4(-2.0f, 0.0f, 0.0f, 2.0f).z, _66.w == float4(-2.0f, 0.0f, 0.0f, 2.0f).w));
    }
    else
    {
        _72 = false;
    }
    bool _76 = false;
    if (_72)
    {
        _76 = true;
    }
    else
    {
        _76 = false;
    }
    bool _83 = false;
    if (_76)
    {
        _83 = all(bool2(float2(-2.0f, 0.0f).x == float4(-2.0f, 0.0f, 0.0f, 2.0f).xy.x, float2(-2.0f, 0.0f).y == float4(-2.0f, 0.0f, 0.0f, 2.0f).xy.y));
    }
    else
    {
        _83 = false;
    }
    bool _90 = false;
    if (_83)
    {
        _90 = all(bool3(float3(-2.0f, 0.0f, 0.0f).x == float4(-2.0f, 0.0f, 0.0f, 2.0f).xyz.x, float3(-2.0f, 0.0f, 0.0f).y == float4(-2.0f, 0.0f, 0.0f, 2.0f).xyz.y, float3(-2.0f, 0.0f, 0.0f).z == float4(-2.0f, 0.0f, 0.0f, 2.0f).xyz.z));
    }
    else
    {
        _90 = false;
    }
    bool _93 = false;
    if (_90)
    {
        _93 = true;
    }
    else
    {
        _93 = false;
    }
    float4 _94 = 0.0f.xxxx;
    if (_93)
    {
        _94 = _11_colorGreen;
    }
    else
    {
        _94 = _11_colorRed;
    }
    return _94;
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
