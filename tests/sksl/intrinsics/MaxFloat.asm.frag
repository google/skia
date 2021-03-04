OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInputs"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpName %_entrypoint "_entrypoint"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %26 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%18 = OpTypeFunction %v4float
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_0_5 = OpConstant %float 0.5
%v2float = OpTypeVector %float 2
%38 = OpConstantComposite %v2float %float_0_5 %float_0_5
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%float_0_75 = OpConstant %float 0.75
%52 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_75
%v3bool = OpTypeVector %bool 3
%float_2_25 = OpConstant %float 2.25
%64 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_75 %float_2_25
%v4bool = OpTypeVector %bool 4
%int_1 = OpConstant %int 1
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%92 = OpConstantComposite %v2float %float_0 %float_1
%105 = OpConstantComposite %v3float %float_0 %float_1 %float_0_75
%116 = OpConstantComposite %v4float %float_0 %float_1 %float_0_75 %float_2_25
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int_2 = OpConstant %int 2
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%120 = OpVariable %_ptr_Function_v4float Function
%22 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%26 = OpLoad %v4float %22
%27 = OpCompositeExtract %float %26 0
%21 = OpExtInst %float %1 FMax %27 %float_0_5
%29 = OpFOrdEqual %bool %21 %float_0_5
OpSelectionMerge %31 None
OpBranchConditional %29 %30 %31
%30 = OpLabel
%33 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%34 = OpLoad %v4float %33
%35 = OpVectorShuffle %v2float %34 %34 0 1
%37 = OpCompositeConstruct %v2float %float_0_5 %float_0_5
%32 = OpExtInst %v2float %1 FMax %35 %37
%39 = OpFOrdEqual %v2bool %32 %38
%41 = OpAll %bool %39
OpBranch %31
%31 = OpLabel
%42 = OpPhi %bool %false %19 %41 %30
OpSelectionMerge %44 None
OpBranchConditional %42 %43 %44
%43 = OpLabel
%46 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%47 = OpLoad %v4float %46
%48 = OpVectorShuffle %v3float %47 %47 0 1 2
%50 = OpCompositeConstruct %v3float %float_0_5 %float_0_5 %float_0_5
%45 = OpExtInst %v3float %1 FMax %48 %50
%53 = OpFOrdEqual %v3bool %45 %52
%55 = OpAll %bool %53
OpBranch %44
%44 = OpLabel
%56 = OpPhi %bool %false %31 %55 %43
OpSelectionMerge %58 None
OpBranchConditional %56 %57 %58
%57 = OpLabel
%60 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%61 = OpLoad %v4float %60
%62 = OpCompositeConstruct %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
%59 = OpExtInst %v4float %1 FMax %61 %62
%65 = OpFOrdEqual %v4bool %59 %64
%67 = OpAll %bool %65
OpBranch %58
%58 = OpLabel
%68 = OpPhi %bool %false %44 %67 %57
OpSelectionMerge %70 None
OpBranchConditional %68 %69 %70
%69 = OpLabel
%72 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%73 = OpLoad %v4float %72
%74 = OpCompositeExtract %float %73 0
%75 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%77 = OpLoad %v4float %75
%78 = OpCompositeExtract %float %77 0
%71 = OpExtInst %float %1 FMax %74 %78
%80 = OpFOrdEqual %bool %71 %float_0
OpBranch %70
%70 = OpLabel
%81 = OpPhi %bool %false %58 %80 %69
OpSelectionMerge %83 None
OpBranchConditional %81 %82 %83
%82 = OpLabel
%85 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%86 = OpLoad %v4float %85
%87 = OpVectorShuffle %v2float %86 %86 0 1
%88 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%89 = OpLoad %v4float %88
%90 = OpVectorShuffle %v2float %89 %89 0 1
%84 = OpExtInst %v2float %1 FMax %87 %90
%93 = OpFOrdEqual %v2bool %84 %92
%94 = OpAll %bool %93
OpBranch %83
%83 = OpLabel
%95 = OpPhi %bool %false %70 %94 %82
OpSelectionMerge %97 None
OpBranchConditional %95 %96 %97
%96 = OpLabel
%99 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%100 = OpLoad %v4float %99
%101 = OpVectorShuffle %v3float %100 %100 0 1 2
%102 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%103 = OpLoad %v4float %102
%104 = OpVectorShuffle %v3float %103 %103 0 1 2
%98 = OpExtInst %v3float %1 FMax %101 %104
%106 = OpFOrdEqual %v3bool %98 %105
%107 = OpAll %bool %106
OpBranch %97
%97 = OpLabel
%108 = OpPhi %bool %false %83 %107 %96
OpSelectionMerge %110 None
OpBranchConditional %108 %109 %110
%109 = OpLabel
%112 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%113 = OpLoad %v4float %112
%114 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%115 = OpLoad %v4float %114
%111 = OpExtInst %v4float %1 FMax %113 %115
%117 = OpFOrdEqual %v4bool %111 %116
%118 = OpAll %bool %117
OpBranch %110
%110 = OpLabel
%119 = OpPhi %bool %false %97 %118 %109
OpSelectionMerge %124 None
OpBranchConditional %119 %122 %123
%122 = OpLabel
%125 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%126 = OpLoad %v4float %125
OpStore %120 %126
OpBranch %124
%123 = OpLabel
%127 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%129 = OpLoad %v4float %127
OpStore %120 %129
OpBranch %124
%124 = OpLabel
%130 = OpLoad %v4float %120
OpReturnValue %130
OpFunctionEnd
