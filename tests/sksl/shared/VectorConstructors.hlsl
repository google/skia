cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _12_colorGreen : packoffset(c0);
    float4 _12_colorRed : packoffset(c1);
    float _12_unknownInput : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

bool check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3i4(float2 _40, float2 _41, float2 _42, float3 _43, int2 _44, int2 _45, float2 _46, float2 _47, float4 _48, int2 _49, bool4 _50, float2 _51, float2 _52, float2 _53, bool2 _54, bool2 _55, bool3 _56, int4 _57)
{
    return (((((((((((((((((_40.x + _41.x) + _42.x) + _43.x) + float(_44.x)) + float(_45.x)) + _46.x) + _47.x) + _48.x) + float(_49.x)) + float(_50.x)) + _51.x) + _52.x) + _53.x) + float(_54.x)) + float(_55.x)) + float(_56.x)) + float(_57.x)) == 18.0f;
}

float4 main(float2 _124)
{
    float2 v1 = 1.0f.xx;
    float2 v2 = float2(1.0f, 2.0f);
    float2 v3 = 1.0f.xx;
    float3 v4 = 1.0f.xxx;
    int2 v5 = int2(1, 1);
    int2 v6 = int2(1, 2);
    float2 v7 = float2(1.0f, 2.0f);
    float2 _144 = float2(float(1), float(1));
    float2 v8 = _144;
    float4 _152 = float4(float(1), _12_unknownInput, 3.0f, 4.0f);
    float4 v9 = _152;
    int2 _156 = int2(3, int(1.0f));
    int2 v10 = _156;
    bool4 v11 = bool4(true, false, true, false);
    float2 v12 = float2(1.0f, 0.0f);
    float2 v13 = 0.0f.xx;
    float2 v14 = 0.0f.xx;
    bool2 v15 = bool2(true, true);
    bool2 v16 = bool2(true, true);
    bool3 v17 = bool3(true, true, true);
    int4 v18 = int4(1, 1, 1, 1);
    float2 _172 = 1.0f.xx;
    float2 _173 = float2(1.0f, 2.0f);
    float2 _174 = 1.0f.xx;
    float3 _175 = 1.0f.xxx;
    int2 _176 = int2(1, 1);
    int2 _177 = int2(1, 2);
    float2 _178 = float2(1.0f, 2.0f);
    float2 _179 = _144;
    float4 _180 = _152;
    int2 _181 = _156;
    bool4 _182 = bool4(true, false, true, false);
    float2 _183 = float2(1.0f, 0.0f);
    float2 _184 = 0.0f.xx;
    float2 _185 = 0.0f.xx;
    bool2 _186 = bool2(true, true);
    bool2 _187 = bool2(true, true);
    bool3 _188 = bool3(true, true, true);
    int4 _189 = int4(1, 1, 1, 1);
    float4 _191 = 0.0f.xxxx;
    if (check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3i4(_172, _173, _174, _175, _176, _177, _178, _179, _180, _181, _182, _183, _184, _185, _186, _187, _188, _189))
    {
        _191 = _12_colorGreen;
    }
    else
    {
        _191 = _12_colorRed;
    }
    return _191;
}

void frag_main()
{
    float2 _22 = 0.0f.xx;
    sk_FragColor = main(_22);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
