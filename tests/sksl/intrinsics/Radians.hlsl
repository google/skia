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
    bool _53 = false;
    if (abs(radians(_11_testInputs.x) - (-0.0218166150152683258056640625f)) < 0.0005000000237487256526947021484375f)
    {
        float2 _44 = abs(radians(_11_testInputs.xy) - float2(-0.0218166150152683258056640625f, 0.0f));
        _53 = all(bool2(_44.x < 0.0005000000237487256526947021484375f.xx.x, _44.y < 0.0005000000237487256526947021484375f.xx.y));
    }
    else
    {
        _53 = false;
    }
    bool _69 = false;
    if (_53)
    {
        float3 _58 = abs(radians(_11_testInputs.xyz) - float3(-0.0218166150152683258056640625f, 0.0f, 0.01308996975421905517578125f));
        _69 = all(bool3(_58.x < 0.0005000000237487256526947021484375f.xxx.x, _58.y < 0.0005000000237487256526947021484375f.xxx.y, _58.z < 0.0005000000237487256526947021484375f.xxx.z));
    }
    else
    {
        _69 = false;
    }
    bool _83 = false;
    if (_69)
    {
        float4 _74 = abs(radians(_11_testInputs) - float4(-0.0218166150152683258056640625f, 0.0f, 0.01308996975421905517578125f, 0.03926990926265716552734375f));
        _83 = all(bool4(_74.x < 0.0005000000237487256526947021484375f.xxxx.x, _74.y < 0.0005000000237487256526947021484375f.xxxx.y, _74.z < 0.0005000000237487256526947021484375f.xxxx.z, _74.w < 0.0005000000237487256526947021484375f.xxxx.w));
    }
    else
    {
        _83 = false;
    }
    float4 _84 = 0.0f.xxxx;
    if (_83)
    {
        _84 = _11_colorGreen;
    }
    else
    {
        _84 = _11_colorRed;
    }
    return _84;
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
