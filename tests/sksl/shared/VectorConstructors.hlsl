cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_colorGreen : packoffset(c0);
    float4 _11_colorRed : packoffset(c1);
    float _11_unknownInput : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3(float2 _37, float2 _38, float2 _39, float3 _40, int2 _41, int2 _42, float2 _43, float2 _44, float4 _45, int2 _46, bool4 _47, float2 _48, float2 _49, float2 _50, bool2 _51, bool2 _52, bool3 _53)
{
    return ((((((((((((((((_37.x + _38.x) + _39.x) + _40.x) + float(_41.x)) + float(_42.x)) + _43.x) + _44.x) + _45.x) + float(_46.x)) + float(_47.x)) + _48.x) + _49.x) + _50.x) + float(_51.x)) + float(_52.x)) + float(_53.x)) == 17.0f;
}

float4 main(float2 _116)
{
    float2 v1 = 1.0f.xx;
    float2 v2 = float2(1.0f, 2.0f);
    float2 v3 = 1.0f.xx;
    float3 v4 = 1.0f.xxx;
    int2 v5 = int2(1, 1);
    int2 v6 = int2(1, 2);
    float2 v7 = float2(1.0f, 2.0f);
    float2 _136 = float2(float(1), float(1));
    float2 v8 = _136;
    float4 _144 = float4(float(1), _11_unknownInput, 3.0f, 4.0f);
    float4 v9 = _144;
    int2 _148 = int2(3, int(1.0f));
    int2 v10 = _148;
    bool4 v11 = bool4(true, false, true, false);
    float2 v12 = float2(1.0f, 0.0f);
    float2 v13 = 0.0f.xx;
    float2 v14 = 0.0f.xx;
    bool2 v15 = bool2(true, true);
    bool2 v16 = bool2(true, true);
    bool3 v17 = bool3(true, true, true);
    float2 _162 = 1.0f.xx;
    float2 _163 = float2(1.0f, 2.0f);
    float2 _164 = 1.0f.xx;
    float3 _165 = 1.0f.xxx;
    int2 _166 = int2(1, 1);
    int2 _167 = int2(1, 2);
    float2 _168 = float2(1.0f, 2.0f);
    float2 _169 = _136;
    float4 _170 = _144;
    int2 _171 = _148;
    bool4 _172 = bool4(true, false, true, false);
    float2 _173 = float2(1.0f, 0.0f);
    float2 _174 = 0.0f.xx;
    float2 _175 = 0.0f.xx;
    bool2 _176 = bool2(true, true);
    bool2 _177 = bool2(true, true);
    bool3 _178 = bool3(true, true, true);
    float4 _180 = 0.0f.xxxx;
    if (check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3(_162, _163, _164, _165, _166, _167, _168, _169, _170, _171, _172, _173, _174, _175, _176, _177, _178))
    {
        _180 = _11_colorGreen;
    }
    else
    {
        _180 = _11_colorRed;
    }
    return _180;
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
