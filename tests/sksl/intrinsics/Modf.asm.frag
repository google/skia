OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %value "value"
OpName %expectedWhole "expectedWhole"
OpName %expectedFraction "expectedFraction"
OpName %ok "ok"
OpName %whole "whole"
OpName %fraction "fraction"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %131 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_2_5 = OpConstant %float 2.5
%float_n2_5 = OpConstant %float -2.5
%float_8 = OpConstant %float 8
%float_n0_125 = OpConstant %float -0.125
%32 = OpConstantComposite %v4float %float_2_5 %float_n2_5 %float_8 %float_n0_125
%float_2 = OpConstant %float 2
%float_n2 = OpConstant %float -2
%36 = OpConstantComposite %v4float %float_2 %float_n2 %float_8 %float_0
%float_0_5 = OpConstant %float 0.5
%float_n0_5 = OpConstant %float -0.5
%40 = OpConstantComposite %v4float %float_0_5 %float_n0_5 %float_0 %float_n0_125
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%false = OpConstantFalse %bool
%45 = OpConstantComposite %v4bool %false %false %false %false
%_ptr_Function_float = OpTypePointer Function %float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_bool = OpTypePointer Function %bool
%77 = OpConstantComposite %v2float %float_2 %float_n2
%v2bool = OpTypeVector %bool 2
%84 = OpConstantComposite %v2float %float_0_5 %float_n0_5
%int_1 = OpConstant %int 1
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%102 = OpConstantComposite %v3float %float_2 %float_n2 %float_8
%v3bool = OpTypeVector %bool 3
%109 = OpConstantComposite %v3float %float_0_5 %float_n0_5 %float_0
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %15
%16 = OpLabel
%20 = OpVariable %_ptr_Function_v2float Function
OpStore %20 %19
%22 = OpFunctionCall %v4float %main %20
OpStore %sk_FragColor %22
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %23
%24 = OpFunctionParameter %_ptr_Function_v2float
%25 = OpLabel
%value = OpVariable %_ptr_Function_v4float Function
%expectedWhole = OpVariable %_ptr_Function_v4float Function
%expectedFraction = OpVariable %_ptr_Function_v4float Function
%ok = OpVariable %_ptr_Function_v4bool Function
%whole = OpVariable %_ptr_Function_v4float Function
%fraction = OpVariable %_ptr_Function_v4float Function
%53 = OpVariable %_ptr_Function_float Function
%70 = OpVariable %_ptr_Function_v2float Function
%94 = OpVariable %_ptr_Function_v3float Function
%117 = OpVariable %_ptr_Function_v4float Function
%132 = OpVariable %_ptr_Function_v4float Function
OpStore %value %32
OpStore %expectedWhole %36
OpStore %expectedFraction %40
OpStore %ok %45
%49 = OpAccessChain %_ptr_Function_float %whole %int_0
%48 = OpExtInst %float %1 Modf %float_2_5 %53
%54 = OpLoad %float %53
OpStore %49 %54
%55 = OpAccessChain %_ptr_Function_float %fraction %int_0
OpStore %55 %48
%56 = OpLoad %v4float %whole
%57 = OpCompositeExtract %float %56 0
%58 = OpFOrdEqual %bool %57 %float_2
OpSelectionMerge %60 None
OpBranchConditional %58 %59 %60
%59 = OpLabel
%61 = OpLoad %v4float %fraction
%62 = OpCompositeExtract %float %61 0
%63 = OpFOrdEqual %bool %62 %float_0_5
OpBranch %60
%60 = OpLabel
%64 = OpPhi %bool %false %25 %63 %59
%65 = OpAccessChain %_ptr_Function_bool %ok %int_0
OpStore %65 %64
%68 = OpLoad %v4float %value
%69 = OpVectorShuffle %v2float %68 %68 0 1
%67 = OpExtInst %v2float %1 Modf %69 %70
%71 = OpLoad %v2float %70
%72 = OpLoad %v4float %whole
%73 = OpVectorShuffle %v4float %72 %71 4 5 2 3
OpStore %whole %73
%74 = OpLoad %v4float %fraction
%75 = OpVectorShuffle %v4float %74 %67 4 5 2 3
OpStore %fraction %75
%76 = OpVectorShuffle %v2float %73 %73 0 1
%78 = OpFOrdEqual %v2bool %76 %77
%80 = OpAll %bool %78
OpSelectionMerge %82 None
OpBranchConditional %80 %81 %82
%81 = OpLabel
%83 = OpVectorShuffle %v2float %75 %75 0 1
%85 = OpFOrdEqual %v2bool %83 %84
%86 = OpAll %bool %85
OpBranch %82
%82 = OpLabel
%87 = OpPhi %bool %false %60 %86 %81
%88 = OpAccessChain %_ptr_Function_bool %ok %int_1
OpStore %88 %87
%91 = OpLoad %v4float %value
%92 = OpVectorShuffle %v3float %91 %91 0 1 2
%90 = OpExtInst %v3float %1 Modf %92 %94
%96 = OpLoad %v3float %94
%97 = OpLoad %v4float %whole
%98 = OpVectorShuffle %v4float %97 %96 4 5 6 3
OpStore %whole %98
%99 = OpLoad %v4float %fraction
%100 = OpVectorShuffle %v4float %99 %90 4 5 6 3
OpStore %fraction %100
%101 = OpVectorShuffle %v3float %98 %98 0 1 2
%103 = OpFOrdEqual %v3bool %101 %102
%105 = OpAll %bool %103
OpSelectionMerge %107 None
OpBranchConditional %105 %106 %107
%106 = OpLabel
%108 = OpVectorShuffle %v3float %100 %100 0 1 2
%110 = OpFOrdEqual %v3bool %108 %109
%111 = OpAll %bool %110
OpBranch %107
%107 = OpLabel
%112 = OpPhi %bool %false %82 %111 %106
%113 = OpAccessChain %_ptr_Function_bool %ok %int_2
OpStore %113 %112
%116 = OpLoad %v4float %value
%115 = OpExtInst %v4float %1 Modf %116 %117
%118 = OpLoad %v4float %117
OpStore %whole %118
OpStore %fraction %115
%119 = OpLoad %v4float %expectedWhole
%120 = OpFOrdEqual %v4bool %118 %119
%121 = OpAll %bool %120
OpSelectionMerge %123 None
OpBranchConditional %121 %122 %123
%122 = OpLabel
%124 = OpLoad %v4float %expectedFraction
%125 = OpFOrdEqual %v4bool %115 %124
%126 = OpAll %bool %125
OpBranch %123
%123 = OpLabel
%127 = OpPhi %bool %false %107 %126 %122
%128 = OpAccessChain %_ptr_Function_bool %ok %int_3
OpStore %128 %127
%131 = OpLoad %v4bool %ok
%130 = OpAll %bool %131
OpSelectionMerge %135 None
OpBranchConditional %130 %133 %134
%133 = OpLabel
%136 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%138 = OpLoad %v4float %136
OpStore %132 %138
OpBranch %135
%134 = OpLabel
%139 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%140 = OpLoad %v4float %139
OpStore %132 %140
OpBranch %135
%135 = OpLabel
%141 = OpLoad %v4float %132
OpReturnValue %141
OpFunctionEnd
