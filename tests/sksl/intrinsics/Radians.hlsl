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
    bool _52 = false;
    if (abs(radians(_10_testInputs.x) - (-0.0218166150152683258056640625f)) < 0.0005000000237487256526947021484375f)
    {
        float2 _43 = abs(radians(_10_testInputs.xy) - float2(-0.0218166150152683258056640625f, 0.0f));
        _52 = all(bool2(_43.x < 0.0005000000237487256526947021484375f.xx.x, _43.y < 0.0005000000237487256526947021484375f.xx.y));
    }
    else
    {
        _52 = false;
    }
    bool _68 = false;
    if (_52)
    {
        float3 _57 = abs(radians(_10_testInputs.xyz) - float3(-0.0218166150152683258056640625f, 0.0f, 0.01308996975421905517578125f));
        _68 = all(bool3(_57.x < 0.0005000000237487256526947021484375f.xxx.x, _57.y < 0.0005000000237487256526947021484375f.xxx.y, _57.z < 0.0005000000237487256526947021484375f.xxx.z));
    }
    else
    {
        _68 = false;
    }
    bool _82 = false;
    if (_68)
    {
        float4 _73 = abs(radians(_10_testInputs) - float4(-0.0218166150152683258056640625f, 0.0f, 0.01308996975421905517578125f, 0.03926990926265716552734375f));
        _82 = all(bool4(_73.x < 0.0005000000237487256526947021484375f.xxxx.x, _73.y < 0.0005000000237487256526947021484375f.xxxx.y, _73.z < 0.0005000000237487256526947021484375f.xxxx.z, _73.w < 0.0005000000237487256526947021484375f.xxxx.w));
    }
    else
    {
        _82 = false;
    }
    float4 _83 = 0.0f.xxxx;
    if (_82)
    {
        _83 = _10_colorGreen;
    }
    else
    {
        _83 = _10_colorRed;
    }
    return _83;
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
