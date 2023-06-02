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
    int check = 0;
    int _38 = 0 + ((_10_colorGreen.y == 1.0f) ? 0 : 1);
    check = _38;
    int _44 = _38 + int(_10_colorGreen.x == 1.0f);
    check = _44;
    int _55 = _44 + (all(bool2(_10_colorGreen.yx.x == _10_colorRed.xy.x, _10_colorGreen.yx.y == _10_colorRed.xy.y)) ? 0 : 1);
    check = _55;
    int _65 = _55 + int(any(bool2(_10_colorGreen.yx.x != _10_colorRed.xy.x, _10_colorGreen.yx.y != _10_colorRed.xy.y)));
    check = _65;
    bool4 _68 = (_65 == 0).xxxx;
    return float4(_68.x ? _10_colorGreen.x : _10_colorRed.x, _68.y ? _10_colorGreen.y : _10_colorRed.y, _68.z ? _10_colorGreen.z : _10_colorRed.z, _68.w ? _10_colorGreen.w : _10_colorRed.w);
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
