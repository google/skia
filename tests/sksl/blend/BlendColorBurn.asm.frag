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
OpDecorate %74 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
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
%float_1 = OpConstant %float 1
%float_0 = OpConstant %float 0
%_ptr_Function_float = OpTypePointer Function %float
%void = OpTypeVoid
%100 = OpTypeFunction %void
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
%74 = OpLoad %v2float %18
%75 = OpCompositeExtract %float %74 1
%76 = OpLoad %float %_6_n
%77 = OpLoad %v2float %17
%78 = OpCompositeExtract %float %77 0
%79 = OpFDiv %float %76 %78
%80 = OpFSub %float %75 %79
%73 = OpExtInst %float %1 FMax %float_0 %80
OpStore %delta %73
%81 = OpLoad %float %delta
%82 = OpLoad %v2float %17
%83 = OpCompositeExtract %float %82 1
%84 = OpFMul %float %81 %83
%85 = OpLoad %v2float %17
%86 = OpCompositeExtract %float %85 0
%87 = OpLoad %v2float %18
%88 = OpCompositeExtract %float %87 1
%89 = OpFSub %float %float_1 %88
%90 = OpFMul %float %86 %89
%91 = OpFAdd %float %84 %90
%92 = OpLoad %v2float %18
%93 = OpCompositeExtract %float %92 0
%94 = OpLoad %v2float %17
%95 = OpCompositeExtract %float %94 1
%96 = OpFSub %float %float_1 %95
%97 = OpFMul %float %93 %96
%98 = OpFAdd %float %91 %97
OpReturnValue %98
%54 = OpLabel
OpBranch %27
%27 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %100
%101 = OpLabel
%_0_blend_color_burn = OpVariable %_ptr_Function_v4float Function
%106 = OpVariable %_ptr_Function_v2float Function
%109 = OpVariable %_ptr_Function_v2float Function
%113 = OpVariable %_ptr_Function_v2float Function
%116 = OpVariable %_ptr_Function_v2float Function
%120 = OpVariable %_ptr_Function_v2float Function
%123 = OpVariable %_ptr_Function_v2float Function
%104 = OpLoad %v4float %src
%105 = OpVectorShuffle %v2float %104 %104 0 3
OpStore %106 %105
%107 = OpLoad %v4float %dst
%108 = OpVectorShuffle %v2float %107 %107 0 3
OpStore %109 %108
%110 = OpFunctionCall %float %_color_burn_component %106 %109
%111 = OpLoad %v4float %src
%112 = OpVectorShuffle %v2float %111 %111 1 3
OpStore %113 %112
%114 = OpLoad %v4float %dst
%115 = OpVectorShuffle %v2float %114 %114 1 3
OpStore %116 %115
%117 = OpFunctionCall %float %_color_burn_component %113 %116
%118 = OpLoad %v4float %src
%119 = OpVectorShuffle %v2float %118 %118 2 3
OpStore %120 %119
%121 = OpLoad %v4float %dst
%122 = OpVectorShuffle %v2float %121 %121 2 3
OpStore %123 %122
%124 = OpFunctionCall %float %_color_burn_component %120 %123
%125 = OpLoad %v4float %src
%126 = OpCompositeExtract %float %125 3
%127 = OpLoad %v4float %src
%128 = OpCompositeExtract %float %127 3
%129 = OpFSub %float %float_1 %128
%130 = OpLoad %v4float %dst
%131 = OpCompositeExtract %float %130 3
%132 = OpFMul %float %129 %131
%133 = OpFAdd %float %126 %132
%134 = OpCompositeConstruct %v4float %110 %117 %124 %133
OpStore %sk_FragColor %134
OpReturn
OpFunctionEnd
