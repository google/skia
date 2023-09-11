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
    bool _50 = false;
    if (abs(radians(_7_testInputs.x) - (-0.0218166150152683258056640625f)) < 0.0005000000237487256526947021484375f)
    {
        float2 _41 = abs(radians(_7_testInputs.xy) - float2(-0.0218166150152683258056640625f, 0.0f));
        _50 = all(bool2(_41.x < 0.0005000000237487256526947021484375f.xx.x, _41.y < 0.0005000000237487256526947021484375f.xx.y));
    }
    else
    {
        _50 = false;
    }
    bool _66 = false;
    if (_50)
    {
        float3 _55 = abs(radians(_7_testInputs.xyz) - float3(-0.0218166150152683258056640625f, 0.0f, 0.01308996975421905517578125f));
        _66 = all(bool3(_55.x < 0.0005000000237487256526947021484375f.xxx.x, _55.y < 0.0005000000237487256526947021484375f.xxx.y, _55.z < 0.0005000000237487256526947021484375f.xxx.z));
    }
    else
    {
        _66 = false;
    }
    bool _80 = false;
    if (_66)
    {
        float4 _71 = abs(radians(_7_testInputs) - float4(-0.0218166150152683258056640625f, 0.0f, 0.01308996975421905517578125f, 0.03926990926265716552734375f));
        _80 = all(bool4(_71.x < 0.0005000000237487256526947021484375f.xxxx.x, _71.y < 0.0005000000237487256526947021484375f.xxxx.y, _71.z < 0.0005000000237487256526947021484375f.xxxx.z, _71.w < 0.0005000000237487256526947021484375f.xxxx.w));
    }
    else
    {
        _80 = false;
    }
    float4 _81 = 0.0f.xxxx;
    if (_80)
    {
        _81 = _7_colorGreen;
    }
    else
    {
        _81 = _7_colorRed;
    }
    return _81;
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
