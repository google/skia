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
OpDecorate %73 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
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
%97 = OpTypeFunction %void
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
%73 = OpLoad %v2float %18
%74 = OpCompositeExtract %float %73 1
%75 = OpLoad %float %_4_n
%76 = OpLoad %float %delta
%77 = OpFDiv %float %75 %76
%72 = OpExtInst %float %1 FMin %74 %77
OpStore %delta %72
%78 = OpLoad %float %delta
%79 = OpLoad %v2float %17
%80 = OpCompositeExtract %float %79 1
%81 = OpFMul %float %78 %80
%82 = OpLoad %v2float %17
%83 = OpCompositeExtract %float %82 0
%84 = OpLoad %v2float %18
%85 = OpCompositeExtract %float %84 1
%86 = OpFSub %float %float_1 %85
%87 = OpFMul %float %83 %86
%88 = OpFAdd %float %81 %87
%89 = OpLoad %v2float %18
%90 = OpCompositeExtract %float %89 0
%91 = OpLoad %v2float %17
%92 = OpCompositeExtract %float %91 1
%93 = OpFSub %float %float_1 %92
%94 = OpFMul %float %90 %93
%95 = OpFAdd %float %88 %94
OpReturnValue %95
%45 = OpLabel
OpBranch %26
%26 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %97
%98 = OpLabel
%_0_blend_color_dodge = OpVariable %_ptr_Function_v4float Function
%103 = OpVariable %_ptr_Function_v2float Function
%106 = OpVariable %_ptr_Function_v2float Function
%110 = OpVariable %_ptr_Function_v2float Function
%113 = OpVariable %_ptr_Function_v2float Function
%117 = OpVariable %_ptr_Function_v2float Function
%120 = OpVariable %_ptr_Function_v2float Function
%101 = OpLoad %v4float %src
%102 = OpVectorShuffle %v2float %101 %101 0 3
OpStore %103 %102
%104 = OpLoad %v4float %dst
%105 = OpVectorShuffle %v2float %104 %104 0 3
OpStore %106 %105
%107 = OpFunctionCall %float %_color_dodge_component %103 %106
%108 = OpLoad %v4float %src
%109 = OpVectorShuffle %v2float %108 %108 1 3
OpStore %110 %109
%111 = OpLoad %v4float %dst
%112 = OpVectorShuffle %v2float %111 %111 1 3
OpStore %113 %112
%114 = OpFunctionCall %float %_color_dodge_component %110 %113
%115 = OpLoad %v4float %src
%116 = OpVectorShuffle %v2float %115 %115 2 3
OpStore %117 %116
%118 = OpLoad %v4float %dst
%119 = OpVectorShuffle %v2float %118 %118 2 3
OpStore %120 %119
%121 = OpFunctionCall %float %_color_dodge_component %117 %120
%122 = OpLoad %v4float %src
%123 = OpCompositeExtract %float %122 3
%124 = OpLoad %v4float %src
%125 = OpCompositeExtract %float %124 3
%126 = OpFSub %float %float_1 %125
%127 = OpLoad %v4float %dst
%128 = OpCompositeExtract %float %127 3
%129 = OpFMul %float %126 %128
%130 = OpFAdd %float %123 %129
%131 = OpCompositeConstruct %v4float %107 %114 %121 %130
OpStore %sk_FragColor %131
OpReturn
OpFunctionEnd
