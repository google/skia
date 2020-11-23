OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise %src %dst
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %src "src"
OpName %dst "dst"
OpName %_color_burn_component "_color_burn_component"
OpName %_5_guarded_divide "_5_guarded_divide"
OpName %_6_n "_6_n"
OpName %delta "delta"
OpName %main "main"
OpName %_0_blend_color_burn "_0_blend_color_burn"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %src RelaxedPrecision
OpDecorate %dst RelaxedPrecision
OpDecorate %20 RelaxedPrecision
OpDecorate %22 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%15 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
%float_1 = OpConstant %float 1
%float_0 = OpConstant %float 0
%_ptr_Function_float = OpTypePointer Function %float
%void = OpTypeVoid
%101 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_color_burn_component = OpFunction %float None %15
%17 = OpFunctionParameter %_ptr_Function_v2float
%18 = OpFunctionParameter %_ptr_Function_v2float
%19 = OpLabel
%_5_guarded_divide = OpVariable %_ptr_Function_float Function
%_6_n = OpVariable %_ptr_Function_float Function
%delta = OpVariable %_ptr_Function_float Function
%20 = OpLoad %v2float %18
%21 = OpCompositeExtract %float %20 1
%22 = OpLoad %v2float %18
%23 = OpCompositeExtract %float %22 0
%24 = OpFOrdEqual %bool %21 %23
OpSelectionMerge %27 None
OpBranchConditional %24 %25 %26
%25 = OpLabel
%28 = OpLoad %v2float %17
%29 = OpCompositeExtract %float %28 1
%30 = OpLoad %v2float %18
%31 = OpCompositeExtract %float %30 1
%32 = OpFMul %float %29 %31
%33 = OpLoad %v2float %17
%34 = OpCompositeExtract %float %33 0
%36 = OpLoad %v2float %18
%37 = OpCompositeExtract %float %36 1
%38 = OpFSub %float %float_1 %37
%39 = OpFMul %float %34 %38
%40 = OpFAdd %float %32 %39
%41 = OpLoad %v2float %18
%42 = OpCompositeExtract %float %41 0
%43 = OpLoad %v2float %17
%44 = OpCompositeExtract %float %43 1
%45 = OpFSub %float %float_1 %44
%46 = OpFMul %float %42 %45
%47 = OpFAdd %float %40 %46
OpReturnValue %47
%26 = OpLabel
%48 = OpLoad %v2float %17
%49 = OpCompositeExtract %float %48 0
%51 = OpFOrdEqual %bool %49 %float_0
OpSelectionMerge %54 None
OpBranchConditional %51 %52 %53
%52 = OpLabel
%55 = OpLoad %v2float %18
%56 = OpCompositeExtract %float %55 0
%57 = OpLoad %v2float %17
%58 = OpCompositeExtract %float %57 1
%59 = OpFSub %float %float_1 %58
%60 = OpFMul %float %56 %59
OpReturnValue %60
%53 = OpLabel
%64 = OpLoad %v2float %18
%65 = OpCompositeExtract %float %64 1
%66 = OpLoad %v2float %18
%67 = OpCompositeExtract %float %66 0
%68 = OpFSub %float %65 %67
%69 = OpLoad %v2float %17
%70 = OpCompositeExtract %float %69 1
%71 = OpFMul %float %68 %70
OpStore %_6_n %71
%72 = OpLoad %float %_6_n
%73 = OpLoad %v2float %17
%74 = OpCompositeExtract %float %73 0
%75 = OpFDiv %float %72 %74
OpStore %_5_guarded_divide %75
%78 = OpLoad %v2float %18
%79 = OpCompositeExtract %float %78 1
%80 = OpLoad %float %_5_guarded_divide
%81 = OpFSub %float %79 %80
%77 = OpExtInst %float %1 FMax %float_0 %81
OpStore %delta %77
%82 = OpLoad %float %delta
%83 = OpLoad %v2float %17
%84 = OpCompositeExtract %float %83 1
%85 = OpFMul %float %82 %84
%86 = OpLoad %v2float %17
%87 = OpCompositeExtract %float %86 0
%88 = OpLoad %v2float %18
%89 = OpCompositeExtract %float %88 1
%90 = OpFSub %float %float_1 %89
%91 = OpFMul %float %87 %90
%92 = OpFAdd %float %85 %91
%93 = OpLoad %v2float %18
%94 = OpCompositeExtract %float %93 0
%95 = OpLoad %v2float %17
%96 = OpCompositeExtract %float %95 1
%97 = OpFSub %float %float_1 %96
%98 = OpFMul %float %94 %97
%99 = OpFAdd %float %92 %98
OpReturnValue %99
%54 = OpLabel
OpBranch %27
%27 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %101
%102 = OpLabel
%_0_blend_color_burn = OpVariable %_ptr_Function_v4float Function
%107 = OpVariable %_ptr_Function_v2float Function
%110 = OpVariable %_ptr_Function_v2float Function
%114 = OpVariable %_ptr_Function_v2float Function
%117 = OpVariable %_ptr_Function_v2float Function
%121 = OpVariable %_ptr_Function_v2float Function
%124 = OpVariable %_ptr_Function_v2float Function
%105 = OpLoad %v4float %src
%106 = OpVectorShuffle %v2float %105 %105 0 3
OpStore %107 %106
%108 = OpLoad %v4float %dst
%109 = OpVectorShuffle %v2float %108 %108 0 3
OpStore %110 %109
%111 = OpFunctionCall %float %_color_burn_component %107 %110
%112 = OpLoad %v4float %src
%113 = OpVectorShuffle %v2float %112 %112 1 3
OpStore %114 %113
%115 = OpLoad %v4float %dst
%116 = OpVectorShuffle %v2float %115 %115 1 3
OpStore %117 %116
%118 = OpFunctionCall %float %_color_burn_component %114 %117
%119 = OpLoad %v4float %src
%120 = OpVectorShuffle %v2float %119 %119 2 3
OpStore %121 %120
%122 = OpLoad %v4float %dst
%123 = OpVectorShuffle %v2float %122 %122 2 3
OpStore %124 %123
%125 = OpFunctionCall %float %_color_burn_component %121 %124
%126 = OpLoad %v4float %src
%127 = OpCompositeExtract %float %126 3
%128 = OpLoad %v4float %src
%129 = OpCompositeExtract %float %128 3
%130 = OpFSub %float %float_1 %129
%131 = OpLoad %v4float %dst
%132 = OpCompositeExtract %float %131 3
%133 = OpFMul %float %130 %132
%134 = OpFAdd %float %127 %133
%135 = OpCompositeConstruct %v4float %111 %118 %125 %134
OpStore %_0_blend_color_burn %135
%136 = OpLoad %v4float %_0_blend_color_burn
OpStore %sk_FragColor %136
OpReturn
OpFunctionEnd
