static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void d_vi(int _27)
{
    int b = 4;
}

void c_vi(int _31)
{
    int _34 = _31;
    d_vi(_34);
}

void b_vi(int _36)
{
    int _39 = _36;
    c_vi(_39);
}

void a_vi(int _41)
{
    int _44 = _41;
    b_vi(_44);
    int _47 = _41;
    b_vi(_47);
}

float4 main(float2 _50)
{
    int i = 0;
    int _54 = i;
    a_vi(_54);
    return 0.0f.xxxx;
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
