OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise %src %dst
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %src "src"
OpName %dst "dst"
OpName %_guarded_divide "_guarded_divide"
OpName %_color_dodge_component "_color_dodge_component"
OpName %delta "delta"
OpName %_4_n "_4_n"
OpName %blend_color_dodge "blend_color_dodge"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %src RelaxedPrecision
OpDecorate %dst RelaxedPrecision
OpDecorate %21 RelaxedPrecision
OpDecorate %22 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_ptr_Input_v4float = OpTypePointer Input %v4float
%src = OpVariable %_ptr_Input_v4float Input
%dst = OpVariable %_ptr_Input_v4float Input
%_ptr_Function_float = OpTypePointer Function %float
%16 = OpTypeFunction %float %_ptr_Function_float %_ptr_Function_float
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%25 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%_ptr_Function_v4float = OpTypePointer Function %v4float
%104 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%void = OpTypeVoid
%141 = OpTypeFunction %void
%_guarded_divide = OpFunction %float None %16
%18 = OpFunctionParameter %_ptr_Function_float
%19 = OpFunctionParameter %_ptr_Function_float
%20 = OpLabel
%21 = OpLoad %float %18
%22 = OpLoad %float %19
%23 = OpFDiv %float %21 %22
OpReturnValue %23
OpFunctionEnd
%_color_dodge_component = OpFunction %float None %25
%27 = OpFunctionParameter %_ptr_Function_v2float
%28 = OpFunctionParameter %_ptr_Function_v2float
%29 = OpLabel
%delta = OpVariable %_ptr_Function_float Function
%_4_n = OpVariable %_ptr_Function_float Function
%30 = OpLoad %v2float %28
%31 = OpCompositeExtract %float %30 0
%33 = OpFOrdEqual %bool %31 %float_0
OpSelectionMerge %36 None
OpBranchConditional %33 %34 %35
%34 = OpLabel
%37 = OpLoad %v2float %27
%38 = OpCompositeExtract %float %37 0
%40 = OpLoad %v2float %28
%41 = OpCompositeExtract %float %40 1
%42 = OpFSub %float %float_1 %41
%43 = OpFMul %float %38 %42
OpReturnValue %43
%35 = OpLabel
%45 = OpLoad %v2float %27
%46 = OpCompositeExtract %float %45 1
%47 = OpLoad %v2float %27
%48 = OpCompositeExtract %float %47 0
%49 = OpFSub %float %46 %48
OpStore %delta %49
%50 = OpLoad %float %delta
%51 = OpFOrdEqual %bool %50 %float_0
OpSelectionMerge %54 None
OpBranchConditional %51 %52 %53
%52 = OpLabel
%55 = OpLoad %v2float %27
%56 = OpCompositeExtract %float %55 1
%57 = OpLoad %v2float %28
%58 = OpCompositeExtract %float %57 1
%59 = OpFMul %float %56 %58
%60 = OpLoad %v2float %27
%61 = OpCompositeExtract %float %60 0
%62 = OpLoad %v2float %28
%63 = OpCompositeExtract %float %62 1
%64 = OpFSub %float %float_1 %63
%65 = OpFMul %float %61 %64
%66 = OpFAdd %float %59 %65
%67 = OpLoad %v2float %28
%68 = OpCompositeExtract %float %67 0
%69 = OpLoad %v2float %27
%70 = OpCompositeExtract %float %69 1
%71 = OpFSub %float %float_1 %70
%72 = OpFMul %float %68 %71
%73 = OpFAdd %float %66 %72
OpReturnValue %73
%53 = OpLabel
%75 = OpLoad %v2float %28
%76 = OpCompositeExtract %float %75 0
%77 = OpLoad %v2float %27
%78 = OpCompositeExtract %float %77 1
%79 = OpFMul %float %76 %78
OpStore %_4_n %79
%81 = OpLoad %v2float %28
%82 = OpCompositeExtract %float %81 1
%83 = OpLoad %float %_4_n
%84 = OpLoad %float %delta
%85 = OpFDiv %float %83 %84
%80 = OpExtInst %float %1 FMin %82 %85
OpStore %delta %80
%86 = OpLoad %float %delta
%87 = OpLoad %v2float %27
%88 = OpCompositeExtract %float %87 1
%89 = OpFMul %float %86 %88
%90 = OpLoad %v2float %27
%91 = OpCompositeExtract %float %90 0
%92 = OpLoad %v2float %28
%93 = OpCompositeExtract %float %92 1
%94 = OpFSub %float %float_1 %93
%95 = OpFMul %float %91 %94
%96 = OpFAdd %float %89 %95
%97 = OpLoad %v2float %28
%98 = OpCompositeExtract %float %97 0
%99 = OpLoad %v2float %27
%100 = OpCompositeExtract %float %99 1
%101 = OpFSub %float %float_1 %100
%102 = OpFMul %float %98 %101
%103 = OpFAdd %float %96 %102
OpReturnValue %103
%54 = OpLabel
OpBranch %36
%36 = OpLabel
OpUnreachable
OpFunctionEnd
%blend_color_dodge = OpFunction %v4float None %104
%106 = OpFunctionParameter %_ptr_Function_v4float
%107 = OpFunctionParameter %_ptr_Function_v4float
%108 = OpLabel
%111 = OpVariable %_ptr_Function_v2float Function
%114 = OpVariable %_ptr_Function_v2float Function
%118 = OpVariable %_ptr_Function_v2float Function
%121 = OpVariable %_ptr_Function_v2float Function
%125 = OpVariable %_ptr_Function_v2float Function
%128 = OpVariable %_ptr_Function_v2float Function
%109 = OpLoad %v4float %106
%110 = OpVectorShuffle %v2float %109 %109 0 3
OpStore %111 %110
%112 = OpLoad %v4float %107
%113 = OpVectorShuffle %v2float %112 %112 0 3
OpStore %114 %113
%115 = OpFunctionCall %float %_color_dodge_component %111 %114
%116 = OpLoad %v4float %106
%117 = OpVectorShuffle %v2float %116 %116 1 3
OpStore %118 %117
%119 = OpLoad %v4float %107
%120 = OpVectorShuffle %v2float %119 %119 1 3
OpStore %121 %120
%122 = OpFunctionCall %float %_color_dodge_component %118 %121
%123 = OpLoad %v4float %106
%124 = OpVectorShuffle %v2float %123 %123 2 3
OpStore %125 %124
%126 = OpLoad %v4float %107
%127 = OpVectorShuffle %v2float %126 %126 2 3
OpStore %128 %127
%129 = OpFunctionCall %float %_color_dodge_component %125 %128
%130 = OpLoad %v4float %106
%131 = OpCompositeExtract %float %130 3
%132 = OpLoad %v4float %106
%133 = OpCompositeExtract %float %132 3
%134 = OpFSub %float %float_1 %133
%135 = OpLoad %v4float %107
%136 = OpCompositeExtract %float %135 3
%137 = OpFMul %float %134 %136
%138 = OpFAdd %float %131 %137
%139 = OpCompositeConstruct %v4float %115 %122 %129 %138
OpReturnValue %139
OpFunctionEnd
%main = OpFunction %void None %141
%142 = OpLabel
%144 = OpVariable %_ptr_Function_v4float Function
%146 = OpVariable %_ptr_Function_v4float Function
%143 = OpLoad %v4float %src
OpStore %144 %143
%145 = OpLoad %v4float %dst
OpStore %146 %145
%147 = OpFunctionCall %v4float %blend_color_dodge %144 %146
OpStore %sk_FragColor %147
OpReturn
OpFunctionEnd
