static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float userfunc_ff(float _20)
{
    return _20 + 1.0f;
}

float4 main(float2 _26)
{
    float b = 2.0f;
    float c = 3.0f;
    b = 2.0f;
    b = 3.0f + 77.0f;
    b = sin(3.0f + 77.0f);
    float _37 = 3.0f + 77.0f;
    float _40 = 3.0f + 77.0f;
    b = userfunc_ff(_40);
    float _42 = cos(3.0f);
    b = _42;
    b = _42;
    for (int x = 0; x < 1; x++)
    {
    }
    float d = c;
    b = 3.0f;
    float _60 = c + 1.0f;
    d = _60;
    return float4(float(3.0f == 2.0f), float(true), float(_60 == 5.0f), float(_60 == 4.0f));
}

void frag_main()
{
    float2 _15 = 0.0f.xx;
    sk_FragColor = main(_15);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
