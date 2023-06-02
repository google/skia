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
    float4 expectedA = float4(0.0f, 0.0f, 0.84375f, 1.0f);
    float4 expectedB = float4(1.0f, 0.0f, 1.0f, 1.0f);
    bool _41 = false;
    if (true)
    {
        _41 = all(bool2(0.0f.xx.x == float4(0.0f, 0.0f, 0.84375f, 1.0f).xy.x, 0.0f.xx.y == float4(0.0f, 0.0f, 0.84375f, 1.0f).xy.y));
    }
    else
    {
        _41 = false;
    }
    bool _50 = false;
    if (_41)
    {
        _50 = all(bool3(float3(0.0f, 0.0f, 0.84375f).x == float4(0.0f, 0.0f, 0.84375f, 1.0f).xyz.x, float3(0.0f, 0.0f, 0.84375f).y == float4(0.0f, 0.0f, 0.84375f, 1.0f).xyz.y, float3(0.0f, 0.0f, 0.84375f).z == float4(0.0f, 0.0f, 0.84375f, 1.0f).xyz.z));
    }
    else
    {
        _50 = false;
    }
    bool _53 = false;
    if (_50)
    {
        _53 = true;
    }
    else
    {
        _53 = false;
    }
    bool _56 = false;
    if (_53)
    {
        _56 = true;
    }
    else
    {
        _56 = false;
    }
    bool _62 = false;
    if (_56)
    {
        _62 = all(bool2(0.0f.xx.x == float4(0.0f, 0.0f, 0.84375f, 1.0f).xy.x, 0.0f.xx.y == float4(0.0f, 0.0f, 0.84375f, 1.0f).xy.y));
    }
    else
    {
        _62 = false;
    }
    bool _68 = false;
    if (_62)
    {
        _68 = all(bool3(float3(0.0f, 0.0f, 0.84375f).x == float4(0.0f, 0.0f, 0.84375f, 1.0f).xyz.x, float3(0.0f, 0.0f, 0.84375f).y == float4(0.0f, 0.0f, 0.84375f, 1.0f).xyz.y, float3(0.0f, 0.0f, 0.84375f).z == float4(0.0f, 0.0f, 0.84375f, 1.0f).xyz.z));
    }
    else
    {
        _68 = false;
    }
    bool _71 = false;
    if (_68)
    {
        _71 = true;
    }
    else
    {
        _71 = false;
    }
    bool _87 = false;
    if (_71)
    {
        _87 = smoothstep(_10_colorRed.y, _10_colorGreen.y, -1.25f) == 0.0f;
    }
    else
    {
        _87 = false;
    }
    bool _103 = false;
    if (_87)
    {
        float2 _90 = smoothstep(_10_colorRed.y.xx, _10_colorGreen.y.xx, float2(-1.25f, 0.0f));
        _103 = all(bool2(_90.x == float4(0.0f, 0.0f, 0.84375f, 1.0f).xy.x, _90.y == float4(0.0f, 0.0f, 0.84375f, 1.0f).xy.y));
    }
    else
    {
        _103 = false;
    }
    bool _120 = false;
    if (_103)
    {
        float3 _106 = smoothstep(_10_colorRed.y.xxx, _10_colorGreen.y.xxx, float3(-1.25f, 0.0f, 0.75f));
        _120 = all(bool3(_106.x == float4(0.0f, 0.0f, 0.84375f, 1.0f).xyz.x, _106.y == float4(0.0f, 0.0f, 0.84375f, 1.0f).xyz.y, _106.z == float4(0.0f, 0.0f, 0.84375f, 1.0f).xyz.z));
    }
    else
    {
        _120 = false;
    }
    bool _137 = false;
    if (_120)
    {
        float4 _123 = smoothstep(_10_colorRed.y.xxxx, _10_colorGreen.y.xxxx, float4(-1.25f, 0.0f, 0.75f, 2.25f));
        _137 = all(bool4(_123.x == float4(0.0f, 0.0f, 0.84375f, 1.0f).x, _123.y == float4(0.0f, 0.0f, 0.84375f, 1.0f).y, _123.z == float4(0.0f, 0.0f, 0.84375f, 1.0f).z, _123.w == float4(0.0f, 0.0f, 0.84375f, 1.0f).w));
    }
    else
    {
        _137 = false;
    }
    bool _140 = false;
    if (_137)
    {
        _140 = true;
    }
    else
    {
        _140 = false;
    }
    bool _147 = false;
    if (_140)
    {
        _147 = all(bool2(float2(1.0f, 0.0f).x == float4(1.0f, 0.0f, 1.0f, 1.0f).xy.x, float2(1.0f, 0.0f).y == float4(1.0f, 0.0f, 1.0f, 1.0f).xy.y));
    }
    else
    {
        _147 = false;
    }
    bool _154 = false;
    if (_147)
    {
        _154 = all(bool3(float3(1.0f, 0.0f, 1.0f).x == float4(1.0f, 0.0f, 1.0f, 1.0f).xyz.x, float3(1.0f, 0.0f, 1.0f).y == float4(1.0f, 0.0f, 1.0f, 1.0f).xyz.y, float3(1.0f, 0.0f, 1.0f).z == float4(1.0f, 0.0f, 1.0f, 1.0f).xyz.z));
    }
    else
    {
        _154 = false;
    }
    bool _157 = false;
    if (_154)
    {
        _157 = true;
    }
    else
    {
        _157 = false;
    }
    bool _168 = false;
    if (_157)
    {
        _168 = smoothstep(_10_colorRed.x, _10_colorGreen.x, -1.25f) == 1.0f;
    }
    else
    {
        _168 = false;
    }
    bool _181 = false;
    if (_168)
    {
        float2 _171 = smoothstep(_10_colorRed.xy, _10_colorGreen.xy, float2(-1.25f, 0.0f));
        _181 = all(bool2(_171.x == float4(1.0f, 0.0f, 1.0f, 1.0f).xy.x, _171.y == float4(1.0f, 0.0f, 1.0f, 1.0f).xy.y));
    }
    else
    {
        _181 = false;
    }
    bool _194 = false;
    if (_181)
    {
        float3 _184 = smoothstep(_10_colorRed.xyz, _10_colorGreen.xyz, float3(-1.25f, 0.0f, 0.75f));
        _194 = all(bool3(_184.x == float4(1.0f, 0.0f, 1.0f, 1.0f).xyz.x, _184.y == float4(1.0f, 0.0f, 1.0f, 1.0f).xyz.y, _184.z == float4(1.0f, 0.0f, 1.0f, 1.0f).xyz.z));
    }
    else
    {
        _194 = false;
    }
    bool _204 = false;
    if (_194)
    {
        float4 _197 = smoothstep(_10_colorRed, _10_colorGreen, float4(-1.25f, 0.0f, 0.75f, 2.25f));
        _204 = all(bool4(_197.x == float4(1.0f, 0.0f, 1.0f, 1.0f).x, _197.y == float4(1.0f, 0.0f, 1.0f, 1.0f).y, _197.z == float4(1.0f, 0.0f, 1.0f, 1.0f).z, _197.w == float4(1.0f, 0.0f, 1.0f, 1.0f).w));
    }
    else
    {
        _204 = false;
    }
    float4 _205 = 0.0f.xxxx;
    if (_204)
    {
        _205 = _10_colorGreen;
    }
    else
    {
        _205 = _10_colorRed;
    }
    return _205;
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
