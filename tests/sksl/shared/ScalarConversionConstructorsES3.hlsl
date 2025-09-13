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
    float f = _11_colorGreen.y;
    int _39 = int(_11_colorGreen.y);
    int i = _39;
    uint _46 = uint(_11_colorGreen.y);
    uint u = _46;
    bool _53 = _11_colorGreen.y != 0.0f;
    bool b = _53;
    float f1 = _11_colorGreen.y;
    float _56 = float(_39);
    float f2 = _56;
    float _58 = float(_46);
    float f3 = _58;
    float _60 = float(_53);
    float f4 = _60;
    int _63 = int(_11_colorGreen.y);
    int i1 = _63;
    int i2 = _39;
    int _66 = int(_46);
    int i3 = _66;
    int _68 = int(_53);
    int i4 = _68;
    uint _71 = uint(_11_colorGreen.y);
    uint u1 = _71;
    uint _73 = uint(_39);
    uint u2 = _73;
    uint u3 = _46;
    uint _76 = uint(_53);
    uint u4 = _76;
    bool _80 = _11_colorGreen.y != 0.0f;
    bool b1 = _80;
    bool _82 = _39 != 0;
    bool b2 = _82;
    bool _84 = _46 != 0u;
    bool b3 = _84;
    bool b4 = _53;
    float4 _115 = 0.0f.xxxx;
    if ((((((((((((((((_11_colorGreen.y + _56) + _58) + _60) + float(_63)) + float(_39)) + float(_66)) + float(_68)) + float(_71)) + float(_73)) + float(_46)) + float(_76)) + float(_80)) + float(_82)) + float(_84)) + float(_53)) == 16.0f)
    {
        _115 = _11_colorGreen;
    }
    else
    {
        _115 = _11_colorRed;
    }
    return _115;
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
