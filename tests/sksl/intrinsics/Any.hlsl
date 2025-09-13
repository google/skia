cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_colorGreen : packoffset(c0);
    float4 _11_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    bool4 _44 = bool4(_11_colorGreen.xxyz.x != 0.0f, _11_colorGreen.xxyz.y != 0.0f, _11_colorGreen.xxyz.z != 0.0f, _11_colorGreen.xxyz.w != 0.0f);
    bool4 inputVal = _44;
    bool4 _57 = bool4(_11_colorGreen.xyyw.x != 0.0f, _11_colorGreen.xyyw.y != 0.0f, _11_colorGreen.xyyw.z != 0.0f, _11_colorGreen.xyyw.w != 0.0f);
    bool4 expected = _57;
    bool _62 = _57.x;
    bool _71 = false;
    if (any(_44.xy) == _62)
    {
        _71 = any(_44.xyz) == _57.y;
    }
    else
    {
        _71 = false;
    }
    bool _77 = false;
    if (_71)
    {
        _77 = any(_44) == _57.z;
    }
    else
    {
        _77 = false;
    }
    bool _81 = false;
    if (_77)
    {
        _81 = false == _62;
    }
    else
    {
        _81 = false;
    }
    bool _85 = false;
    if (_81)
    {
        _85 = _57.y;
    }
    else
    {
        _85 = false;
    }
    bool _89 = false;
    if (_85)
    {
        _89 = _57.z;
    }
    else
    {
        _89 = false;
    }
    float4 _90 = 0.0f.xxxx;
    if (_89)
    {
        _90 = _11_colorGreen;
    }
    else
    {
        _90 = _11_colorRed;
    }
    return _90;
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
