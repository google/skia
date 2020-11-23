OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise %src %dst
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %src "src"
OpName %dst "dst"
OpName %_color_dodge_component "_color_dodge_component"
OpName %delta "delta"
OpName %_3_guarded_divide "_3_guarded_divide"
OpName %_4_n "_4_n"
OpName %main "main"
OpName %_0_blend_color_dodge "_0_blend_color_dodge"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %src RelaxedPrecision
OpDecorate %dst RelaxedPrecision
OpDecorate %20 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
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
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%_ptr_Function_float = OpTypePointer Function %float
%void = OpTypeVoid
%98 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_color_dodge_component = OpFunction %float None %15
%17 = OpFunctionParameter %_ptr_Function_v2float
%18 = OpFunctionParameter %_ptr_Function_v2float
%19 = OpLabel
%delta = OpVariable %_ptr_Function_float Function
%_3_guarded_divide = OpVariable %_ptr_Function_float Function
%_4_n = OpVariable %_ptr_Function_float Function
%20 = OpLoad %v2float %18
%21 = OpCompositeExtract %float %20 0
%23 = OpFOrdEqual %bool %21 %float_0
OpSelectionMerge %26 None
OpBranchConditional %23 %24 %25
%24 = OpLabel
%27 = OpLoad %v2float %17
%28 = OpCompositeExtract %float %27 0
%30 = OpLoad %v2float %18
%31 = OpCompositeExtract %float %30 1
%32 = OpFSub %float %float_1 %31
%33 = OpFMul %float %28 %32
OpReturnValue %33
%25 = OpLabel
%36 = OpLoad %v2float %17
%37 = OpCompositeExtract %float %36 1
%38 = OpLoad %v2float %17
%39 = OpCompositeExtract %float %38 0
%40 = OpFSub %float %37 %39
OpStore %delta %40
%41 = OpLoad %float %delta
%42 = OpFOrdEqual %bool %41 %float_0
OpSelectionMerge %45 None
OpBranchConditional %42 %43 %44
%43 = OpLabel
%46 = OpLoad %v2float %17
%47 = OpCompositeExtract %float %46 1
%48 = OpLoad %v2float %18
%49 = OpCompositeExtract %float %48 1
%50 = OpFMul %float %47 %49
%51 = OpLoad %v2float %17
%52 = OpCompositeExtract %float %51 0
%53 = OpLoad %v2float %18
%54 = OpCompositeExtract %float %53 1
%55 = OpFSub %float %float_1 %54
%56 = OpFMul %float %52 %55
%57 = OpFAdd %float %50 %56
%58 = OpLoad %v2float %18
%59 = OpCompositeExtract %float %58 0
%60 = OpLoad %v2float %17
%61 = OpCompositeExtract %float %60 1
%62 = OpFSub %float %float_1 %61
%63 = OpFMul %float %59 %62
%64 = OpFAdd %float %57 %63
OpReturnValue %64
%44 = OpLabel
%67 = OpLoad %v2float %18
%68 = OpCompositeExtract %float %67 0
%69 = OpLoad %v2float %17
%70 = OpCompositeExtract %float %69 1
%71 = OpFMul %float %68 %70
OpStore %_4_n %71
%72 = OpLoad %float %_4_n
%73 = OpLoad %float %delta
%74 = OpFDiv %float %72 %73
OpStore %_3_guarded_divide %74
%76 = OpLoad %v2float %18
%77 = OpCompositeExtract %float %76 1
%78 = OpLoad %float %_3_guarded_divide
%75 = OpExtInst %float %1 FMin %77 %78
OpStore %delta %75
%79 = OpLoad %float %delta
%80 = OpLoad %v2float %17
%81 = OpCompositeExtract %float %80 1
%82 = OpFMul %float %79 %81
%83 = OpLoad %v2float %17
%84 = OpCompositeExtract %float %83 0
%85 = OpLoad %v2float %18
%86 = OpCompositeExtract %float %85 1
%87 = OpFSub %float %float_1 %86
%88 = OpFMul %float %84 %87
%89 = OpFAdd %float %82 %88
%90 = OpLoad %v2float %18
%91 = OpCompositeExtract %float %90 0
%92 = OpLoad %v2float %17
%93 = OpCompositeExtract %float %92 1
%94 = OpFSub %float %float_1 %93
%95 = OpFMul %float %91 %94
%96 = OpFAdd %float %89 %95
OpReturnValue %96
%45 = OpLabel
OpBranch %26
%26 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %98
%99 = OpLabel
%_0_blend_color_dodge = OpVariable %_ptr_Function_v4float Function
%104 = OpVariable %_ptr_Function_v2float Function
%107 = OpVariable %_ptr_Function_v2float Function
%111 = OpVariable %_ptr_Function_v2float Function
%114 = OpVariable %_ptr_Function_v2float Function
%118 = OpVariable %_ptr_Function_v2float Function
%121 = OpVariable %_ptr_Function_v2float Function
%102 = OpLoad %v4float %src
%103 = OpVectorShuffle %v2float %102 %102 0 3
OpStore %104 %103
%105 = OpLoad %v4float %dst
%106 = OpVectorShuffle %v2float %105 %105 0 3
OpStore %107 %106
%108 = OpFunctionCall %float %_color_dodge_component %104 %107
%109 = OpLoad %v4float %src
%110 = OpVectorShuffle %v2float %109 %109 1 3
OpStore %111 %110
%112 = OpLoad %v4float %dst
%113 = OpVectorShuffle %v2float %112 %112 1 3
OpStore %114 %113
%115 = OpFunctionCall %float %_color_dodge_component %111 %114
%116 = OpLoad %v4float %src
%117 = OpVectorShuffle %v2float %116 %116 2 3
OpStore %118 %117
%119 = OpLoad %v4float %dst
%120 = OpVectorShuffle %v2float %119 %119 2 3
OpStore %121 %120
%122 = OpFunctionCall %float %_color_dodge_component %118 %121
%123 = OpLoad %v4float %src
%124 = OpCompositeExtract %float %123 3
%125 = OpLoad %v4float %src
%126 = OpCompositeExtract %float %125 3
%127 = OpFSub %float %float_1 %126
%128 = OpLoad %v4float %dst
%129 = OpCompositeExtract %float %128 3
%130 = OpFMul %float %127 %129
%131 = OpFAdd %float %124 %130
%132 = OpCompositeConstruct %v4float %108 %115 %122 %131
OpStore %_0_blend_color_dodge %132
%133 = OpLoad %v4float %_0_blend_color_dodge
OpStore %sk_FragColor %133
OpReturn
OpFunctionEnd
