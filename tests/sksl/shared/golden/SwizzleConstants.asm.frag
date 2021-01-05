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
OpDecorate %21 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
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
%20 = OpCompositeConstruct %v4float %19 %float_1 %float_1 %float_1
OpStore %sk_FragColor %20
%21 = OpLoad %v4float %v
%22 = OpVectorShuffle %v2float %21 %21 0 1
%24 = OpCompositeExtract %float %22 0
%25 = OpCompositeExtract %float %22 1
%26 = OpCompositeConstruct %v4float %24 %25 %float_1 %float_1
OpStore %sk_FragColor %26
%27 = OpLoad %v4float %v
%28 = OpCompositeExtract %float %27 0
%29 = OpCompositeConstruct %v4float %28 %float_1 %float_1 %float_1
OpStore %sk_FragColor %29
%31 = OpLoad %v4float %v
%32 = OpCompositeExtract %float %31 1
%33 = OpCompositeConstruct %v4float %float_0 %32 %float_1 %float_1
OpStore %sk_FragColor %33
%34 = OpLoad %v4float %v
%35 = OpVectorShuffle %v3float %34 %34 0 1 2
%37 = OpCompositeExtract %float %35 0
%38 = OpCompositeExtract %float %35 1
%39 = OpCompositeExtract %float %35 2
%40 = OpCompositeConstruct %v4float %37 %38 %39 %float_1
OpStore %sk_FragColor %40
%41 = OpLoad %v4float %v
%42 = OpVectorShuffle %v2float %41 %41 0 1
%43 = OpCompositeExtract %float %42 0
%44 = OpCompositeExtract %float %42 1
%45 = OpCompositeConstruct %v4float %43 %44 %float_1 %float_1
OpStore %sk_FragColor %45
%46 = OpLoad %v4float %v
%47 = OpCompositeExtract %float %46 0
%48 = OpLoad %v4float %v
%49 = OpCompositeExtract %float %48 2
%50 = OpCompositeConstruct %v4float %47 %float_0 %49 %float_1
OpStore %sk_FragColor %50
%51 = OpLoad %v4float %v
%52 = OpCompositeExtract %float %51 0
%53 = OpCompositeConstruct %v4float %52 %float_1 %float_0 %float_1
OpStore %sk_FragColor %53
%54 = OpLoad %v4float %v
%55 = OpVectorShuffle %v2float %54 %54 1 2
%56 = OpCompositeExtract %float %55 0
%57 = OpCompositeExtract %float %55 1
%58 = OpCompositeConstruct %v4float %float_1 %56 %57 %float_1
OpStore %sk_FragColor %58
%59 = OpLoad %v4float %v
%60 = OpCompositeExtract %float %59 1
%61 = OpCompositeConstruct %v4float %float_0 %60 %float_1 %float_1
OpStore %sk_FragColor %61
%62 = OpLoad %v4float %v
%63 = OpCompositeExtract %float %62 2
%64 = OpCompositeConstruct %v4float %float_1 %float_1 %63 %float_1
OpStore %sk_FragColor %64
%65 = OpLoad %v4float %v
OpStore %sk_FragColor %65
%66 = OpLoad %v4float %v
%67 = OpVectorShuffle %v3float %66 %66 0 1 2
%68 = OpCompositeExtract %float %67 0
%69 = OpCompositeExtract %float %67 1
%70 = OpCompositeExtract %float %67 2
%71 = OpCompositeConstruct %v4float %68 %69 %70 %float_1
OpStore %sk_FragColor %71
%72 = OpLoad %v4float %v
%73 = OpVectorShuffle %v2float %72 %72 0 1
%74 = OpCompositeExtract %float %73 0
%75 = OpCompositeExtract %float %73 1
%76 = OpLoad %v4float %v
%77 = OpCompositeExtract %float %76 3
%78 = OpCompositeConstruct %v4float %74 %75 %float_0 %77
OpStore %sk_FragColor %78
%79 = OpLoad %v4float %v
%80 = OpVectorShuffle %v2float %79 %79 0 1
%81 = OpCompositeExtract %float %80 0
%82 = OpCompositeExtract %float %80 1
%83 = OpCompositeConstruct %v4float %81 %82 %float_1 %float_0
OpStore %sk_FragColor %83
%84 = OpLoad %v4float %v
%85 = OpCompositeExtract %float %84 0
%86 = OpLoad %v4float %v
%87 = OpVectorShuffle %v2float %86 %86 2 3
%88 = OpCompositeExtract %float %87 0
%89 = OpCompositeExtract %float %87 1
%90 = OpCompositeConstruct %v4float %85 %float_1 %88 %89
OpStore %sk_FragColor %90
%91 = OpLoad %v4float %v
%92 = OpCompositeExtract %float %91 0
%93 = OpLoad %v4float %v
%94 = OpCompositeExtract %float %93 2
%95 = OpCompositeConstruct %v4float %92 %float_0 %94 %float_1
OpStore %sk_FragColor %95
%96 = OpLoad %v4float %v
%97 = OpCompositeExtract %float %96 0
%98 = OpLoad %v4float %v
%99 = OpCompositeExtract %float %98 3
%100 = OpCompositeConstruct %v4float %97 %float_1 %float_1 %99
OpStore %sk_FragColor %100
%101 = OpLoad %v4float %v
%102 = OpCompositeExtract %float %101 0
%103 = OpCompositeConstruct %v4float %102 %float_1 %float_0 %float_1
OpStore %sk_FragColor %103
%104 = OpLoad %v4float %v
%105 = OpVectorShuffle %v3float %104 %104 1 2 3
%106 = OpCompositeExtract %float %105 0
%107 = OpCompositeExtract %float %105 1
%108 = OpCompositeExtract %float %105 2
%109 = OpCompositeConstruct %v4float %float_1 %106 %107 %108
OpStore %sk_FragColor %109
%110 = OpLoad %v4float %v
%111 = OpVectorShuffle %v2float %110 %110 1 2
%112 = OpCompositeExtract %float %111 0
%113 = OpCompositeExtract %float %111 1
%114 = OpCompositeConstruct %v4float %float_0 %112 %113 %float_1
OpStore %sk_FragColor %114
%115 = OpLoad %v4float %v
%116 = OpCompositeExtract %float %115 1
%117 = OpLoad %v4float %v
%118 = OpCompositeExtract %float %117 3
%119 = OpCompositeConstruct %v4float %float_0 %116 %float_1 %118
OpStore %sk_FragColor %119
%120 = OpLoad %v4float %v
%121 = OpCompositeExtract %float %120 1
%122 = OpCompositeConstruct %v4float %float_1 %121 %float_1 %float_1
OpStore %sk_FragColor %122
%123 = OpLoad %v4float %v
%124 = OpVectorShuffle %v2float %123 %123 2 3
%125 = OpCompositeExtract %float %124 0
%126 = OpCompositeExtract %float %124 1
%127 = OpCompositeConstruct %v4float %float_0 %float_0 %125 %126
OpStore %sk_FragColor %127
%128 = OpLoad %v4float %v
%129 = OpCompositeExtract %float %128 2
%130 = OpCompositeConstruct %v4float %float_0 %float_0 %129 %float_1
OpStore %sk_FragColor %130
%131 = OpLoad %v4float %v
%132 = OpCompositeExtract %float %131 3
%133 = OpCompositeConstruct %v4float %float_0 %float_1 %float_1 %132
OpStore %sk_FragColor %133
OpReturn
OpFunctionEnd
