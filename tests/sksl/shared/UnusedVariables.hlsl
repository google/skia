static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float userfunc_ff(float _23)
{
    return _23 + 1.0f;
}

float4 main(float2 _29)
{
    float b = 2.0f;
    float c = 3.0f;
    b = 2.0f;
    b = 3.0f + 77.0f;
    b = sin(3.0f + 77.0f);
    float _40 = 3.0f + 77.0f;
    float _43 = 3.0f + 77.0f;
    b = userfunc_ff(_43);
    float _45 = cos(3.0f);
    b = _45;
    b = _45;
    for (int x = 0; x < 1; x++)
    {
    }
    float d = c;
    b = 3.0f;
    float _62 = c + 1.0f;
    d = _62;
    return float4(float(3.0f == 2.0f), float(true), float(_62 == 5.0f), float(_62 == 4.0f));
}

void frag_main()
{
    float2 _18 = 0.0f.xx;
    sk_FragColor = main(_18);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
