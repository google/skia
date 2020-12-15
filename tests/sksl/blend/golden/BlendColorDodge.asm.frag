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
OpName %_4_n "_4_n"
OpName %main "main"
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
OpDecorate %66 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
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
%96 = OpTypeFunction %void
%_color_dodge_component = OpFunction %float None %15
%17 = OpFunctionParameter %_ptr_Function_v2float
%18 = OpFunctionParameter %_ptr_Function_v2float
%19 = OpLabel
%delta = OpVariable %_ptr_Function_float Function
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
%66 = OpLoad %v2float %18
%67 = OpCompositeExtract %float %66 0
%68 = OpLoad %v2float %17
%69 = OpCompositeExtract %float %68 1
%70 = OpFMul %float %67 %69
OpStore %_4_n %70
%72 = OpLoad %v2float %18
%73 = OpCompositeExtract %float %72 1
%74 = OpLoad %float %_4_n
%75 = OpLoad %float %delta
%76 = OpFDiv %float %74 %75
%71 = OpExtInst %float %1 FMin %73 %76
OpStore %delta %71
%77 = OpLoad %float %delta
%78 = OpLoad %v2float %17
%79 = OpCompositeExtract %float %78 1
%80 = OpFMul %float %77 %79
%81 = OpLoad %v2float %17
%82 = OpCompositeExtract %float %81 0
%83 = OpLoad %v2float %18
%84 = OpCompositeExtract %float %83 1
%85 = OpFSub %float %float_1 %84
%86 = OpFMul %float %82 %85
%87 = OpFAdd %float %80 %86
%88 = OpLoad %v2float %18
%89 = OpCompositeExtract %float %88 0
%90 = OpLoad %v2float %17
%91 = OpCompositeExtract %float %90 1
%92 = OpFSub %float %float_1 %91
%93 = OpFMul %float %89 %92
%94 = OpFAdd %float %87 %93
OpReturnValue %94
%45 = OpLabel
OpBranch %26
%26 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %void None %96
%97 = OpLabel
%100 = OpVariable %_ptr_Function_v2float Function
%103 = OpVariable %_ptr_Function_v2float Function
%107 = OpVariable %_ptr_Function_v2float Function
%110 = OpVariable %_ptr_Function_v2float Function
%114 = OpVariable %_ptr_Function_v2float Function
%117 = OpVariable %_ptr_Function_v2float Function
%98 = OpLoad %v4float %src
%99 = OpVectorShuffle %v2float %98 %98 0 3
OpStore %100 %99
%101 = OpLoad %v4float %dst
%102 = OpVectorShuffle %v2float %101 %101 0 3
OpStore %103 %102
%104 = OpFunctionCall %float %_color_dodge_component %100 %103
%105 = OpLoad %v4float %src
%106 = OpVectorShuffle %v2float %105 %105 1 3
OpStore %107 %106
%108 = OpLoad %v4float %dst
%109 = OpVectorShuffle %v2float %108 %108 1 3
OpStore %110 %109
%111 = OpFunctionCall %float %_color_dodge_component %107 %110
%112 = OpLoad %v4float %src
%113 = OpVectorShuffle %v2float %112 %112 2 3
OpStore %114 %113
%115 = OpLoad %v4float %dst
%116 = OpVectorShuffle %v2float %115 %115 2 3
OpStore %117 %116
%118 = OpFunctionCall %float %_color_dodge_component %114 %117
%119 = OpLoad %v4float %src
%120 = OpCompositeExtract %float %119 3
%121 = OpLoad %v4float %src
%122 = OpCompositeExtract %float %121 3
%123 = OpFSub %float %float_1 %122
%124 = OpLoad %v4float %dst
%125 = OpCompositeExtract %float %124 3
%126 = OpFMul %float %123 %125
%127 = OpFAdd %float %120 %126
%128 = OpCompositeConstruct %v4float %104 %111 %118 %127
OpStore %sk_FragColor %128
OpReturn
OpFunctionEnd
