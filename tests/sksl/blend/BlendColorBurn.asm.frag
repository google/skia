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
OpName %_6_n "_6_n"
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
OpDecorate %63 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
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
%float_1 = OpConstant %float 1
%float_0 = OpConstant %float 0
%_ptr_Function_float = OpTypePointer Function %float
%void = OpTypeVoid
%99 = OpTypeFunction %void
%_color_burn_component = OpFunction %float None %15
%17 = OpFunctionParameter %_ptr_Function_v2float
%18 = OpFunctionParameter %_ptr_Function_v2float
%19 = OpLabel
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
%63 = OpLoad %v2float %18
%64 = OpCompositeExtract %float %63 1
%65 = OpLoad %v2float %18
%66 = OpCompositeExtract %float %65 0
%67 = OpFSub %float %64 %66
%68 = OpLoad %v2float %17
%69 = OpCompositeExtract %float %68 1
%70 = OpFMul %float %67 %69
OpStore %_6_n %70
%73 = OpLoad %v2float %18
%74 = OpCompositeExtract %float %73 1
%75 = OpLoad %float %_6_n
%76 = OpLoad %v2float %17
%77 = OpCompositeExtract %float %76 0
%78 = OpFDiv %float %75 %77
%79 = OpFSub %float %74 %78
%72 = OpExtInst %float %1 FMax %float_0 %79
OpStore %delta %72
%80 = OpLoad %float %delta
%81 = OpLoad %v2float %17
%82 = OpCompositeExtract %float %81 1
%83 = OpFMul %float %80 %82
%84 = OpLoad %v2float %17
%85 = OpCompositeExtract %float %84 0
%86 = OpLoad %v2float %18
%87 = OpCompositeExtract %float %86 1
%88 = OpFSub %float %float_1 %87
%89 = OpFMul %float %85 %88
%90 = OpFAdd %float %83 %89
%91 = OpLoad %v2float %18
%92 = OpCompositeExtract %float %91 0
%93 = OpLoad %v2float %17
%94 = OpCompositeExtract %float %93 1
%95 = OpFSub %float %float_1 %94
%96 = OpFMul %float %92 %95
%97 = OpFAdd %float %90 %96
OpReturnValue %97
%54 = OpLabel
OpBranch %27
%27 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %99
%100 = OpLabel
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
%107 = OpFunctionCall %float %_color_burn_component %103 %106
%108 = OpLoad %v4float %src
%109 = OpVectorShuffle %v2float %108 %108 1 3
OpStore %110 %109
%111 = OpLoad %v4float %dst
%112 = OpVectorShuffle %v2float %111 %111 1 3
OpStore %113 %112
%114 = OpFunctionCall %float %_color_burn_component %110 %113
%115 = OpLoad %v4float %src
%116 = OpVectorShuffle %v2float %115 %115 2 3
OpStore %117 %116
%118 = OpLoad %v4float %dst
%119 = OpVectorShuffle %v2float %118 %118 2 3
OpStore %120 %119
%121 = OpFunctionCall %float %_color_burn_component %117 %120
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
