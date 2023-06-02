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

bool verify_const_globals_bii(int _27, int _28)
{
    bool _39 = false;
    if (_27 == 7)
    {
        _39 = _28 == 10;
    }
    else
    {
        _39 = false;
    }
    return _39;
}

float4 main(float2 _41)
{
    int _43 = 7;
    int _44 = 10;
    bool4 _47 = verify_const_globals_bii(_43, _44).xxxx;
    return float4(_47.x ? _11_colorGreen.x : _11_colorRed.x, _47.y ? _11_colorGreen.y : _11_colorRed.y, _47.z ? _11_colorGreen.z : _11_colorRed.z, _47.w ? _11_colorGreen.w : _11_colorRed.w);
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
