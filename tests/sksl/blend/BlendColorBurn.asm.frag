### Compilation failed:

error: SPIR-V validation error: Variable must be decorated with a location
  %src = OpVariable %_ptr_Input_v4float Input

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
OpName %_color_burn_component "_color_burn_component"
OpName %delta "delta"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %src RelaxedPrecision
OpDecorate %dst RelaxedPrecision
OpDecorate %20 RelaxedPrecision
OpDecorate %21 RelaxedPrecision
OpDecorate %22 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
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
%15 = OpTypeFunction %float %_ptr_Function_float %_ptr_Function_float
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
%float_1 = OpConstant %float 1
%float_0 = OpConstant %float 0
%void = OpTypeVoid
%107 = OpTypeFunction %void
%_guarded_divide = OpFunction %float None %15
%17 = OpFunctionParameter %_ptr_Function_float
%18 = OpFunctionParameter %_ptr_Function_float
%19 = OpLabel
%20 = OpLoad %float %17
%21 = OpLoad %float %18
%22 = OpFDiv %float %20 %21
OpReturnValue %22
OpFunctionEnd
%_color_burn_component = OpFunction %float None %24
%26 = OpFunctionParameter %_ptr_Function_v2float
%27 = OpFunctionParameter %_ptr_Function_v2float
%28 = OpLabel
%delta = OpVariable %_ptr_Function_float Function
%82 = OpVariable %_ptr_Function_float Function
%85 = OpVariable %_ptr_Function_float Function
%29 = OpLoad %v2float %27
%30 = OpCompositeExtract %float %29 1
%31 = OpLoad %v2float %27
%32 = OpCompositeExtract %float %31 0
%33 = OpFOrdEqual %bool %30 %32
OpSelectionMerge %36 None
OpBranchConditional %33 %34 %35
%34 = OpLabel
%37 = OpLoad %v2float %26
%38 = OpCompositeExtract %float %37 1
%39 = OpLoad %v2float %27
%40 = OpCompositeExtract %float %39 1
%41 = OpFMul %float %38 %40
%42 = OpLoad %v2float %26
%43 = OpCompositeExtract %float %42 0
%45 = OpLoad %v2float %27
%46 = OpCompositeExtract %float %45 1
%47 = OpFSub %float %float_1 %46
%48 = OpFMul %float %43 %47
%49 = OpFAdd %float %41 %48
%50 = OpLoad %v2float %27
%51 = OpCompositeExtract %float %50 0
%52 = OpLoad %v2float %26
%53 = OpCompositeExtract %float %52 1
%54 = OpFSub %float %float_1 %53
%55 = OpFMul %float %51 %54
%56 = OpFAdd %float %49 %55
OpReturnValue %56
%35 = OpLabel
%57 = OpLoad %v2float %26
%58 = OpCompositeExtract %float %57 0
%60 = OpFOrdEqual %bool %58 %float_0
OpSelectionMerge %63 None
OpBranchConditional %60 %61 %62
%61 = OpLabel
%64 = OpLoad %v2float %27
%65 = OpCompositeExtract %float %64 0
%66 = OpLoad %v2float %26
%67 = OpCompositeExtract %float %66 1
%68 = OpFSub %float %float_1 %67
%69 = OpFMul %float %65 %68
OpReturnValue %69
%62 = OpLabel
%72 = OpLoad %v2float %27
%73 = OpCompositeExtract %float %72 1
%74 = OpLoad %v2float %27
%75 = OpCompositeExtract %float %74 1
%76 = OpLoad %v2float %27
%77 = OpCompositeExtract %float %76 0
%78 = OpFSub %float %75 %77
%79 = OpLoad %v2float %26
%80 = OpCompositeExtract %float %79 1
%81 = OpFMul %float %78 %80
OpStore %82 %81
%83 = OpLoad %v2float %26
%84 = OpCompositeExtract %float %83 0
OpStore %85 %84
%86 = OpFunctionCall %float %_guarded_divide %82 %85
%87 = OpFSub %float %73 %86
%71 = OpExtInst %float %1 FMax %float_0 %87
OpStore %delta %71
%88 = OpLoad %float %delta
%89 = OpLoad %v2float %26
%90 = OpCompositeExtract %float %89 1
%91 = OpFMul %float %88 %90
%92 = OpLoad %v2float %26
%93 = OpCompositeExtract %float %92 0
%94 = OpLoad %v2float %27
%95 = OpCompositeExtract %float %94 1
%96 = OpFSub %float %float_1 %95
%97 = OpFMul %float %93 %96
%98 = OpFAdd %float %91 %97
%99 = OpLoad %v2float %27
%100 = OpCompositeExtract %float %99 0
%101 = OpLoad %v2float %26
%102 = OpCompositeExtract %float %101 1
%103 = OpFSub %float %float_1 %102
%104 = OpFMul %float %100 %103
%105 = OpFAdd %float %98 %104
OpReturnValue %105
%63 = OpLabel
OpBranch %36
%36 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %107
%108 = OpLabel
%111 = OpVariable %_ptr_Function_v2float Function
%114 = OpVariable %_ptr_Function_v2float Function
%118 = OpVariable %_ptr_Function_v2float Function
%121 = OpVariable %_ptr_Function_v2float Function
%125 = OpVariable %_ptr_Function_v2float Function
%128 = OpVariable %_ptr_Function_v2float Function
%109 = OpLoad %v4float %src
%110 = OpVectorShuffle %v2float %109 %109 0 3
OpStore %111 %110
%112 = OpLoad %v4float %dst
%113 = OpVectorShuffle %v2float %112 %112 0 3
OpStore %114 %113
%115 = OpFunctionCall %float %_color_burn_component %111 %114
%116 = OpLoad %v4float %src
%117 = OpVectorShuffle %v2float %116 %116 1 3
OpStore %118 %117
%119 = OpLoad %v4float %dst
%120 = OpVectorShuffle %v2float %119 %119 1 3
OpStore %121 %120
%122 = OpFunctionCall %float %_color_burn_component %118 %121
%123 = OpLoad %v4float %src
%124 = OpVectorShuffle %v2float %123 %123 2 3
OpStore %125 %124
%126 = OpLoad %v4float %dst
%127 = OpVectorShuffle %v2float %126 %126 2 3
OpStore %128 %127
%129 = OpFunctionCall %float %_color_burn_component %125 %128
%130 = OpLoad %v4float %src
%131 = OpCompositeExtract %float %130 3
%132 = OpLoad %v4float %src
%133 = OpCompositeExtract %float %132 3
%134 = OpFSub %float %float_1 %133
%135 = OpLoad %v4float %dst
%136 = OpCompositeExtract %float %135 3
%137 = OpFMul %float %134 %136
%138 = OpFAdd %float %131 %137
%139 = OpCompositeConstruct %v4float %115 %122 %129 %138
OpStore %sk_FragColor %139
OpReturn
OpFunctionEnd

1 error
