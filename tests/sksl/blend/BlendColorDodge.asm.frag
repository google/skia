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
OpName %_color_dodge_component "_color_dodge_component"
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
OpDecorate %36 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
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
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%void = OpTypeVoid
%104 = OpTypeFunction %void
%_guarded_divide = OpFunction %float None %15
%17 = OpFunctionParameter %_ptr_Function_float
%18 = OpFunctionParameter %_ptr_Function_float
%19 = OpLabel
%20 = OpLoad %float %17
%21 = OpLoad %float %18
%22 = OpFDiv %float %20 %21
OpReturnValue %22
OpFunctionEnd
%_color_dodge_component = OpFunction %float None %24
%26 = OpFunctionParameter %_ptr_Function_v2float
%27 = OpFunctionParameter %_ptr_Function_v2float
%28 = OpLabel
%delta = OpVariable %_ptr_Function_float Function
%81 = OpVariable %_ptr_Function_float Function
%83 = OpVariable %_ptr_Function_float Function
%29 = OpLoad %v2float %27
%30 = OpCompositeExtract %float %29 0
%32 = OpFOrdEqual %bool %30 %float_0
OpSelectionMerge %35 None
OpBranchConditional %32 %33 %34
%33 = OpLabel
%36 = OpLoad %v2float %26
%37 = OpCompositeExtract %float %36 0
%39 = OpLoad %v2float %27
%40 = OpCompositeExtract %float %39 1
%41 = OpFSub %float %float_1 %40
%42 = OpFMul %float %37 %41
OpReturnValue %42
%34 = OpLabel
%44 = OpLoad %v2float %26
%45 = OpCompositeExtract %float %44 1
%46 = OpLoad %v2float %26
%47 = OpCompositeExtract %float %46 0
%48 = OpFSub %float %45 %47
OpStore %delta %48
%49 = OpLoad %float %delta
%50 = OpFOrdEqual %bool %49 %float_0
OpSelectionMerge %53 None
OpBranchConditional %50 %51 %52
%51 = OpLabel
%54 = OpLoad %v2float %26
%55 = OpCompositeExtract %float %54 1
%56 = OpLoad %v2float %27
%57 = OpCompositeExtract %float %56 1
%58 = OpFMul %float %55 %57
%59 = OpLoad %v2float %26
%60 = OpCompositeExtract %float %59 0
%61 = OpLoad %v2float %27
%62 = OpCompositeExtract %float %61 1
%63 = OpFSub %float %float_1 %62
%64 = OpFMul %float %60 %63
%65 = OpFAdd %float %58 %64
%66 = OpLoad %v2float %27
%67 = OpCompositeExtract %float %66 0
%68 = OpLoad %v2float %26
%69 = OpCompositeExtract %float %68 1
%70 = OpFSub %float %float_1 %69
%71 = OpFMul %float %67 %70
%72 = OpFAdd %float %65 %71
OpReturnValue %72
%52 = OpLabel
%74 = OpLoad %v2float %27
%75 = OpCompositeExtract %float %74 1
%76 = OpLoad %v2float %27
%77 = OpCompositeExtract %float %76 0
%78 = OpLoad %v2float %26
%79 = OpCompositeExtract %float %78 1
%80 = OpFMul %float %77 %79
OpStore %81 %80
%82 = OpLoad %float %delta
OpStore %83 %82
%84 = OpFunctionCall %float %_guarded_divide %81 %83
%73 = OpExtInst %float %1 FMin %75 %84
OpStore %delta %73
%85 = OpLoad %float %delta
%86 = OpLoad %v2float %26
%87 = OpCompositeExtract %float %86 1
%88 = OpFMul %float %85 %87
%89 = OpLoad %v2float %26
%90 = OpCompositeExtract %float %89 0
%91 = OpLoad %v2float %27
%92 = OpCompositeExtract %float %91 1
%93 = OpFSub %float %float_1 %92
%94 = OpFMul %float %90 %93
%95 = OpFAdd %float %88 %94
%96 = OpLoad %v2float %27
%97 = OpCompositeExtract %float %96 0
%98 = OpLoad %v2float %26
%99 = OpCompositeExtract %float %98 1
%100 = OpFSub %float %float_1 %99
%101 = OpFMul %float %97 %100
%102 = OpFAdd %float %95 %101
OpReturnValue %102
%53 = OpLabel
OpBranch %35
%35 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %104
%105 = OpLabel
%108 = OpVariable %_ptr_Function_v2float Function
%111 = OpVariable %_ptr_Function_v2float Function
%115 = OpVariable %_ptr_Function_v2float Function
%118 = OpVariable %_ptr_Function_v2float Function
%122 = OpVariable %_ptr_Function_v2float Function
%125 = OpVariable %_ptr_Function_v2float Function
%106 = OpLoad %v4float %src
%107 = OpVectorShuffle %v2float %106 %106 0 3
OpStore %108 %107
%109 = OpLoad %v4float %dst
%110 = OpVectorShuffle %v2float %109 %109 0 3
OpStore %111 %110
%112 = OpFunctionCall %float %_color_dodge_component %108 %111
%113 = OpLoad %v4float %src
%114 = OpVectorShuffle %v2float %113 %113 1 3
OpStore %115 %114
%116 = OpLoad %v4float %dst
%117 = OpVectorShuffle %v2float %116 %116 1 3
OpStore %118 %117
%119 = OpFunctionCall %float %_color_dodge_component %115 %118
%120 = OpLoad %v4float %src
%121 = OpVectorShuffle %v2float %120 %120 2 3
OpStore %122 %121
%123 = OpLoad %v4float %dst
%124 = OpVectorShuffle %v2float %123 %123 2 3
OpStore %125 %124
%126 = OpFunctionCall %float %_color_dodge_component %122 %125
%127 = OpLoad %v4float %src
%128 = OpCompositeExtract %float %127 3
%129 = OpLoad %v4float %src
%130 = OpCompositeExtract %float %129 3
%131 = OpFSub %float %float_1 %130
%132 = OpLoad %v4float %dst
%133 = OpCompositeExtract %float %132 3
%134 = OpFMul %float %131 %133
%135 = OpFAdd %float %128 %134
%136 = OpCompositeConstruct %v4float %112 %119 %126 %135
OpStore %sk_FragColor %136
OpReturn
OpFunctionEnd

1 error
