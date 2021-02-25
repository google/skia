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
OpDecorate %39 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
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
%float_n1 = OpConstant %float -1
%float_1 = OpConstant %float 1
%float_0 = OpConstant %float 0
%float_0_75 = OpConstant %float 0.75
%32 = OpConstantComposite %v4float %float_n1 %float_0 %float_0_75 %float_1
%v2float = OpTypeVector %float 2
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%float_n2 = OpConstant %float -2
%81 = OpConstantComposite %v4float %float_n1 %float_n2 %float_n2 %float_1
%float_2 = OpConstant %float 2
%float_0_5 = OpConstant %float 0.5
%float_3 = OpConstant %float 3
%86 = OpConstantComposite %v4float %float_1 %float_2 %float_0_5 %float_3
%float_2_25 = OpConstant %float 2.25
%89 = OpConstantComposite %v4float %float_n1 %float_0 %float_0_5 %float_2_25
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%125 = OpVariable %_ptr_Function_v4float Function
%22 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%26 = OpLoad %v4float %22
%27 = OpCompositeExtract %float %26 0
%21 = OpExtInst %float %1 FClamp %27 %float_n1 %float_1
%33 = OpCompositeExtract %float %32 0
%34 = OpFOrdEqual %bool %21 %33
OpSelectionMerge %36 None
OpBranchConditional %34 %35 %36
%35 = OpLabel
%38 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%39 = OpLoad %v4float %38
%40 = OpVectorShuffle %v2float %39 %39 0 1
%42 = OpCompositeConstruct %v2float %float_n1 %float_n1
%43 = OpCompositeConstruct %v2float %float_1 %float_1
%37 = OpExtInst %v2float %1 FClamp %40 %42 %43
%44 = OpVectorShuffle %v2float %32 %32 0 1
%45 = OpFOrdEqual %v2bool %37 %44
%47 = OpAll %bool %45
OpBranch %36
%36 = OpLabel
%48 = OpPhi %bool %false %19 %47 %35
OpSelectionMerge %50 None
OpBranchConditional %48 %49 %50
%49 = OpLabel
%52 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%53 = OpLoad %v4float %52
%54 = OpVectorShuffle %v3float %53 %53 0 1 2
%56 = OpCompositeConstruct %v3float %float_n1 %float_n1 %float_n1
%57 = OpCompositeConstruct %v3float %float_1 %float_1 %float_1
%51 = OpExtInst %v3float %1 FClamp %54 %56 %57
%58 = OpVectorShuffle %v3float %32 %32 0 1 2
%59 = OpFOrdEqual %v3bool %51 %58
%61 = OpAll %bool %59
OpBranch %50
%50 = OpLabel
%62 = OpPhi %bool %false %36 %61 %49
OpSelectionMerge %64 None
OpBranchConditional %62 %63 %64
%63 = OpLabel
%66 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%67 = OpLoad %v4float %66
%68 = OpCompositeConstruct %v4float %float_n1 %float_n1 %float_n1 %float_n1
%69 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%65 = OpExtInst %v4float %1 FClamp %67 %68 %69
%70 = OpFOrdEqual %v4bool %65 %32
%72 = OpAll %bool %70
OpBranch %64
%64 = OpLabel
%73 = OpPhi %bool %false %50 %72 %63
OpSelectionMerge %75 None
OpBranchConditional %73 %74 %75
%74 = OpLabel
%77 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%78 = OpLoad %v4float %77
%79 = OpCompositeExtract %float %78 0
%82 = OpCompositeExtract %float %81 0
%87 = OpCompositeExtract %float %86 0
%76 = OpExtInst %float %1 FClamp %79 %82 %87
%90 = OpCompositeExtract %float %89 0
%91 = OpFOrdEqual %bool %76 %90
OpBranch %75
%75 = OpLabel
%92 = OpPhi %bool %false %64 %91 %74
OpSelectionMerge %94 None
OpBranchConditional %92 %93 %94
%93 = OpLabel
%96 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%97 = OpLoad %v4float %96
%98 = OpVectorShuffle %v2float %97 %97 0 1
%99 = OpVectorShuffle %v2float %81 %81 0 1
%100 = OpVectorShuffle %v2float %86 %86 0 1
%95 = OpExtInst %v2float %1 FClamp %98 %99 %100
%101 = OpVectorShuffle %v2float %89 %89 0 1
%102 = OpFOrdEqual %v2bool %95 %101
%103 = OpAll %bool %102
OpBranch %94
%94 = OpLabel
%104 = OpPhi %bool %false %75 %103 %93
OpSelectionMerge %106 None
OpBranchConditional %104 %105 %106
%105 = OpLabel
%108 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%109 = OpLoad %v4float %108
%110 = OpVectorShuffle %v3float %109 %109 0 1 2
%111 = OpVectorShuffle %v3float %81 %81 0 1 2
%112 = OpVectorShuffle %v3float %86 %86 0 1 2
%107 = OpExtInst %v3float %1 FClamp %110 %111 %112
%113 = OpVectorShuffle %v3float %89 %89 0 1 2
%114 = OpFOrdEqual %v3bool %107 %113
%115 = OpAll %bool %114
OpBranch %106
%106 = OpLabel
%116 = OpPhi %bool %false %94 %115 %105
OpSelectionMerge %118 None
OpBranchConditional %116 %117 %118
%117 = OpLabel
%120 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%121 = OpLoad %v4float %120
%119 = OpExtInst %v4float %1 FClamp %121 %81 %86
%122 = OpFOrdEqual %v4bool %119 %89
%123 = OpAll %bool %122
OpBranch %118
%118 = OpLabel
%124 = OpPhi %bool %false %106 %123 %117
OpSelectionMerge %129 None
OpBranchConditional %124 %127 %128
%127 = OpLabel
%130 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%132 = OpLoad %v4float %130
OpStore %125 %132
OpBranch %129
%128 = OpLabel
%133 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%135 = OpLoad %v4float %133
OpStore %125 %135
OpBranch %129
%129 = OpLabel
%136 = OpLoad %v4float %125
OpReturnValue %136
OpFunctionEnd
