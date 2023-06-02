cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorGreen : packoffset(c0);
    float4 _10_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    int a = 0;
    int b = 0;
    int c = 0;
    int d = 0;
    a = 1;
    b = 2;
    c = 5;
    bool _40 = false;
    if (true)
    {
        _40 = true;
    }
    else
    {
        _40 = false;
    }
    bool _43 = false;
    if (_40)
    {
        _43 = true;
    }
    else
    {
        _43 = false;
    }
    bool _46 = false;
    if (_43)
    {
        _46 = true;
    }
    else
    {
        _46 = false;
    }
    bool4 _48 = _46.xxxx;
    return float4(_48.x ? _10_colorGreen.x : _10_colorRed.x, _48.y ? _10_colorGreen.y : _10_colorRed.y, _48.z ? _10_colorGreen.z : _10_colorRed.z, _48.w ? _10_colorGreen.w : _10_colorRed.w);
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
