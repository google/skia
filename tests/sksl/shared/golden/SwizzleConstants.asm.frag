OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpName %v "v"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %18 RelaxedPrecision
OpDecorate %22 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%11 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_1 = OpConstant %float 1
%float_1_0 = OpConstant %float 1
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%v3float = OpTypeVector %float 3
%main = OpFunction %void None %11
%12 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%15 = OpExtInst %float %1 Sqrt %float_1
%17 = OpCompositeConstruct %v4float %15 %15 %15 %15
OpStore %v %17
%18 = OpLoad %v4float %v
%19 = OpCompositeExtract %float %18 0
%21 = OpCompositeConstruct %v4float %19 %float_1_0 %float_1_0 %float_1_0
OpStore %sk_FragColor %21
%22 = OpLoad %v4float %v
%23 = OpVectorShuffle %v2float %22 %22 0 1
%25 = OpCompositeExtract %float %23 0
%26 = OpCompositeExtract %float %23 1
%27 = OpCompositeConstruct %v4float %25 %26 %float_1_0 %float_1_0
OpStore %sk_FragColor %27
%28 = OpLoad %v4float %v
%29 = OpCompositeExtract %float %28 0
%30 = OpCompositeConstruct %v4float %29 %float_1_0 %float_1_0 %float_1_0
OpStore %sk_FragColor %30
%32 = OpLoad %v4float %v
%33 = OpCompositeExtract %float %32 1
%34 = OpCompositeConstruct %v4float %float_0 %33 %float_1_0 %float_1_0
OpStore %sk_FragColor %34
%35 = OpLoad %v4float %v
%36 = OpVectorShuffle %v3float %35 %35 0 1 2
%38 = OpCompositeExtract %float %36 0
%39 = OpCompositeExtract %float %36 1
%40 = OpCompositeExtract %float %36 2
%41 = OpCompositeConstruct %v4float %38 %39 %40 %float_1_0
OpStore %sk_FragColor %41
%42 = OpLoad %v4float %v
%43 = OpVectorShuffle %v2float %42 %42 0 1
%44 = OpCompositeExtract %float %43 0
%45 = OpCompositeExtract %float %43 1
%46 = OpCompositeConstruct %v4float %44 %45 %float_1_0 %float_1_0
OpStore %sk_FragColor %46
%47 = OpLoad %v4float %v
%48 = OpCompositeExtract %float %47 0
%49 = OpLoad %v4float %v
%50 = OpCompositeExtract %float %49 2
%51 = OpCompositeConstruct %v4float %48 %float_0 %50 %float_1_0
OpStore %sk_FragColor %51
%52 = OpLoad %v4float %v
%53 = OpCompositeExtract %float %52 0
%54 = OpCompositeConstruct %v4float %53 %float_1_0 %float_0 %float_1_0
OpStore %sk_FragColor %54
%55 = OpLoad %v4float %v
%56 = OpVectorShuffle %v2float %55 %55 1 2
%57 = OpCompositeExtract %float %56 0
%58 = OpCompositeExtract %float %56 1
%59 = OpCompositeConstruct %v4float %float_1_0 %57 %58 %float_1_0
OpStore %sk_FragColor %59
%60 = OpLoad %v4float %v
%61 = OpCompositeExtract %float %60 1
%62 = OpCompositeConstruct %v4float %float_0 %61 %float_1_0 %float_1_0
OpStore %sk_FragColor %62
%63 = OpLoad %v4float %v
%64 = OpCompositeExtract %float %63 2
%65 = OpCompositeConstruct %v4float %float_1_0 %float_1_0 %64 %float_1_0
OpStore %sk_FragColor %65
%66 = OpLoad %v4float %v
OpStore %sk_FragColor %66
%67 = OpLoad %v4float %v
%68 = OpVectorShuffle %v3float %67 %67 0 1 2
%69 = OpCompositeExtract %float %68 0
%70 = OpCompositeExtract %float %68 1
%71 = OpCompositeExtract %float %68 2
%72 = OpCompositeConstruct %v4float %69 %70 %71 %float_1_0
OpStore %sk_FragColor %72
%73 = OpLoad %v4float %v
%74 = OpVectorShuffle %v2float %73 %73 0 1
%75 = OpCompositeExtract %float %74 0
%76 = OpCompositeExtract %float %74 1
%77 = OpLoad %v4float %v
%78 = OpCompositeExtract %float %77 3
%79 = OpCompositeConstruct %v4float %75 %76 %float_0 %78
OpStore %sk_FragColor %79
%80 = OpLoad %v4float %v
%81 = OpVectorShuffle %v2float %80 %80 0 1
%82 = OpCompositeExtract %float %81 0
%83 = OpCompositeExtract %float %81 1
%84 = OpCompositeConstruct %v4float %82 %83 %float_1_0 %float_0
OpStore %sk_FragColor %84
%85 = OpLoad %v4float %v
%86 = OpCompositeExtract %float %85 0
%87 = OpLoad %v4float %v
%88 = OpVectorShuffle %v2float %87 %87 2 3
%89 = OpCompositeExtract %float %88 0
%90 = OpCompositeExtract %float %88 1
%91 = OpCompositeConstruct %v4float %86 %float_1_0 %89 %90
OpStore %sk_FragColor %91
%92 = OpLoad %v4float %v
%93 = OpCompositeExtract %float %92 0
%94 = OpLoad %v4float %v
%95 = OpCompositeExtract %float %94 2
%96 = OpCompositeConstruct %v4float %93 %float_0 %95 %float_1_0
OpStore %sk_FragColor %96
%97 = OpLoad %v4float %v
%98 = OpCompositeExtract %float %97 0
%99 = OpLoad %v4float %v
%100 = OpCompositeExtract %float %99 3
%101 = OpCompositeConstruct %v4float %98 %float_1_0 %float_1_0 %100
OpStore %sk_FragColor %101
%102 = OpLoad %v4float %v
%103 = OpCompositeExtract %float %102 0
%104 = OpCompositeConstruct %v4float %103 %float_1_0 %float_0 %float_1_0
OpStore %sk_FragColor %104
%105 = OpLoad %v4float %v
%106 = OpVectorShuffle %v3float %105 %105 1 2 3
%107 = OpCompositeExtract %float %106 0
%108 = OpCompositeExtract %float %106 1
%109 = OpCompositeExtract %float %106 2
%110 = OpCompositeConstruct %v4float %float_1_0 %107 %108 %109
OpStore %sk_FragColor %110
%111 = OpLoad %v4float %v
%112 = OpVectorShuffle %v2float %111 %111 1 2
%113 = OpCompositeExtract %float %112 0
%114 = OpCompositeExtract %float %112 1
%115 = OpCompositeConstruct %v4float %float_0 %113 %114 %float_1_0
OpStore %sk_FragColor %115
%116 = OpLoad %v4float %v
%117 = OpCompositeExtract %float %116 1
%118 = OpLoad %v4float %v
%119 = OpCompositeExtract %float %118 3
%120 = OpCompositeConstruct %v4float %float_0 %117 %float_1_0 %119
OpStore %sk_FragColor %120
%121 = OpLoad %v4float %v
%122 = OpCompositeExtract %float %121 1
%123 = OpCompositeConstruct %v4float %float_1_0 %122 %float_1_0 %float_1_0
OpStore %sk_FragColor %123
%124 = OpLoad %v4float %v
%125 = OpVectorShuffle %v2float %124 %124 2 3
%126 = OpCompositeExtract %float %125 0
%127 = OpCompositeExtract %float %125 1
%128 = OpCompositeConstruct %v4float %float_0 %float_0 %126 %127
OpStore %sk_FragColor %128
%129 = OpLoad %v4float %v
%130 = OpCompositeExtract %float %129 2
%131 = OpCompositeConstruct %v4float %float_0 %float_0 %130 %float_1_0
OpStore %sk_FragColor %131
%132 = OpLoad %v4float %v
%133 = OpCompositeExtract %float %132 3
%134 = OpCompositeConstruct %v4float %float_0 %float_1_0 %float_1_0 %133
OpStore %sk_FragColor %134
OpReturn
OpFunctionEnd
