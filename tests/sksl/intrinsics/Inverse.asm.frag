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
OpName %inv2x2 "inv2x2"
OpName %inv3x3 "inv3x3"
OpName %inv4x4 "inv4x4"
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
OpDecorate %inv2x2 RelaxedPrecision
OpDecorate %inv3x3 RelaxedPrecision
OpDecorate %inv4x4 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
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
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_n2 = OpConstant %float -2
%float_1 = OpConstant %float 1
%float_1_5 = OpConstant %float 1.5
%float_n0_5 = OpConstant %float -0.5
%33 = OpConstantComposite %v2float %float_n2 %float_1
%34 = OpConstantComposite %v2float %float_1_5 %float_n0_5
%35 = OpConstantComposite %mat2v2float %33 %34
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_n24 = OpConstant %float -24
%float_18 = OpConstant %float 18
%float_5 = OpConstant %float 5
%float_20 = OpConstant %float 20
%float_n15 = OpConstant %float -15
%float_n4 = OpConstant %float -4
%float_n5 = OpConstant %float -5
%float_4 = OpConstant %float 4
%48 = OpConstantComposite %v3float %float_n24 %float_18 %float_5
%49 = OpConstantComposite %v3float %float_20 %float_n15 %float_n4
%50 = OpConstantComposite %v3float %float_n5 %float_4 %float_1
%51 = OpConstantComposite %mat3v3float %48 %49 %50
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%float_0_5 = OpConstant %float 0.5
%float_n8 = OpConstant %float -8
%float_n1 = OpConstant %float -1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%60 = OpConstantComposite %v4float %float_n2 %float_n0_5 %float_1 %float_0_5
%61 = OpConstantComposite %v4float %float_1 %float_0_5 %float_0 %float_n0_5
%62 = OpConstantComposite %v4float %float_n8 %float_n1 %float_2 %float_2
%63 = OpConstantComposite %v4float %float_3 %float_0_5 %float_n1 %float_n0_5
%64 = OpConstantComposite %mat4v4float %60 %61 %62 %63
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%float_6 = OpConstant %float 6
%float_7 = OpConstant %float 7
%float_8 = OpConstant %float 8
%float_9 = OpConstant %float 9
%106 = OpConstantComposite %v3float %float_1 %float_2 %float_3
%107 = OpConstantComposite %v3float %float_4 %float_5 %float_6
%108 = OpConstantComposite %v3float %float_7 %float_8 %float_9
%109 = OpConstantComposite %mat3v3float %106 %107 %108
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
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
%inv2x2 = OpVariable %_ptr_Function_mat2v2float Function
%inv3x3 = OpVariable %_ptr_Function_mat3v3float Function
%inv4x4 = OpVariable %_ptr_Function_mat4v4float Function
%122 = OpVariable %_ptr_Function_v4float Function
OpStore %inv2x2 %35
OpStore %inv3x3 %51
OpStore %inv4x4 %64
%67 = OpFOrdEqual %v2bool %33 %33
%68 = OpAll %bool %67
%69 = OpFOrdEqual %v2bool %34 %34
%70 = OpAll %bool %69
%71 = OpLogicalAnd %bool %68 %70
OpSelectionMerge %73 None
OpBranchConditional %71 %72 %73
%72 = OpLabel
%75 = OpFOrdEqual %v3bool %48 %48
%76 = OpAll %bool %75
%77 = OpFOrdEqual %v3bool %49 %49
%78 = OpAll %bool %77
%79 = OpLogicalAnd %bool %76 %78
%80 = OpFOrdEqual %v3bool %50 %50
%81 = OpAll %bool %80
%82 = OpLogicalAnd %bool %79 %81
OpBranch %73
%73 = OpLabel
%83 = OpPhi %bool %false %25 %82 %72
OpSelectionMerge %85 None
OpBranchConditional %83 %84 %85
%84 = OpLabel
%87 = OpFOrdEqual %v4bool %60 %60
%88 = OpAll %bool %87
%89 = OpFOrdEqual %v4bool %61 %61
%90 = OpAll %bool %89
%91 = OpLogicalAnd %bool %88 %90
%92 = OpFOrdEqual %v4bool %62 %62
%93 = OpAll %bool %92
%94 = OpLogicalAnd %bool %91 %93
%95 = OpFOrdEqual %v4bool %63 %63
%96 = OpAll %bool %95
%97 = OpLogicalAnd %bool %94 %96
OpBranch %85
%85 = OpLabel
%98 = OpPhi %bool %false %73 %97 %84
OpSelectionMerge %100 None
OpBranchConditional %98 %99 %100
%99 = OpLabel
%101 = OpExtInst %mat3v3float %1 MatrixInverse %109
%110 = OpCompositeExtract %v3float %101 0
%111 = OpFUnordNotEqual %v3bool %110 %48
%112 = OpAny %bool %111
%113 = OpCompositeExtract %v3float %101 1
%114 = OpFUnordNotEqual %v3bool %113 %49
%115 = OpAny %bool %114
%116 = OpLogicalOr %bool %112 %115
%117 = OpCompositeExtract %v3float %101 2
%118 = OpFUnordNotEqual %v3bool %117 %50
%119 = OpAny %bool %118
%120 = OpLogicalOr %bool %116 %119
OpBranch %100
%100 = OpLabel
%121 = OpPhi %bool %false %85 %120 %99
OpSelectionMerge %126 None
OpBranchConditional %121 %124 %125
%124 = OpLabel
%127 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%131 = OpLoad %v4float %127
OpStore %122 %131
OpBranch %126
%125 = OpLabel
%132 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%134 = OpLoad %v4float %132
OpStore %122 %134
OpBranch %126
%126 = OpLabel
%135 = OpLoad %v4float %122
OpReturnValue %135
OpFunctionEnd
