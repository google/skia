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
OpMemberName %_UniformBuffer 2 "testMatrix2x2"
OpMemberName %_UniformBuffer 3 "testMatrix3x3"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %h22 "h22"
OpName %f22 "f22"
OpName %h33 "h33"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 ColMajor
OpMemberDecorate %_UniformBuffer 2 MatrixStride 16
OpMemberDecorate %_UniformBuffer 3 Offset 64
OpMemberDecorate %_UniformBuffer 3 ColMajor
OpMemberDecorate %_UniformBuffer 3 MatrixStride 16
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %h22 RelaxedPrecision
OpDecorate %h33 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float %mat3v3float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%19 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%26 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_5 = OpConstant %float 5
%float_10 = OpConstant %float 10
%float_15 = OpConstant %float 15
%34 = OpConstantComposite %v2float %float_0 %float_5
%35 = OpConstantComposite %v2float %float_10 %float_15
%36 = OpConstantComposite %mat2v2float %34 %35
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%float_1 = OpConstant %float 1
%46 = OpConstantComposite %v2float %float_1 %float_0
%47 = OpConstantComposite %v2float %float_0 %float_1
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
%int_3 = OpConstant %int 3
%float_2 = OpConstant %float 2
%63 = OpConstantComposite %v3float %float_2 %float_2 %float_2
%64 = OpConstantComposite %mat3v3float %63 %63 %63
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%float_4 = OpConstant %float 4
%86 = OpConstantComposite %v2float %float_0 %float_4
%87 = OpConstantComposite %mat2v2float %46 %86
%float_6 = OpConstant %float 6
%float_8 = OpConstant %float 8
%float_12 = OpConstant %float 12
%float_14 = OpConstant %float 14
%float_16 = OpConstant %float 16
%float_18 = OpConstant %float 18
%105 = OpConstantComposite %v3float %float_2 %float_4 %float_6
%106 = OpConstantComposite %v3float %float_8 %float_10 %float_12
%107 = OpConstantComposite %v3float %float_14 %float_16 %float_18
%108 = OpConstantComposite %mat3v3float %105 %106 %107
%v3bool = OpTypeVector %bool 3
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %19
%20 = OpLabel
%23 = OpVariable %_ptr_Function_v2float Function
OpStore %23 %22
%25 = OpFunctionCall %v4float %main %23
OpStore %sk_FragColor %25
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %26
%27 = OpFunctionParameter %_ptr_Function_v2float
%28 = OpLabel
%h22 = OpVariable %_ptr_Function_mat2v2float Function
%f22 = OpVariable %_ptr_Function_mat2v2float Function
%h33 = OpVariable %_ptr_Function_mat3v3float Function
%122 = OpVariable %_ptr_Function_v4float Function
OpStore %h22 %36
%39 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%43 = OpLoad %mat2v2float %39
%45 = OpCompositeConstruct %mat2v2float %46 %47
%48 = OpCompositeExtract %v2float %43 0
%49 = OpCompositeExtract %v2float %45 0
%50 = OpFMul %v2float %48 %49
%51 = OpCompositeExtract %v2float %43 1
%52 = OpCompositeExtract %v2float %45 1
%53 = OpFMul %v2float %51 %52
%54 = OpCompositeConstruct %mat2v2float %50 %53
OpStore %f22 %54
%58 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%61 = OpLoad %mat3v3float %58
%65 = OpCompositeExtract %v3float %61 0
%66 = OpFMul %v3float %65 %63
%67 = OpCompositeExtract %v3float %61 1
%68 = OpFMul %v3float %67 %63
%69 = OpCompositeExtract %v3float %61 2
%70 = OpFMul %v3float %69 %63
%71 = OpCompositeConstruct %mat3v3float %66 %68 %70
OpStore %h33 %71
%73 = OpLoad %mat2v2float %h22
%75 = OpCompositeExtract %v2float %73 0
%76 = OpFOrdEqual %v2bool %75 %34
%77 = OpAll %bool %76
%78 = OpCompositeExtract %v2float %73 1
%79 = OpFOrdEqual %v2bool %78 %35
%80 = OpAll %bool %79
%81 = OpLogicalAnd %bool %77 %80
OpSelectionMerge %83 None
OpBranchConditional %81 %82 %83
%82 = OpLabel
%84 = OpLoad %mat2v2float %f22
%88 = OpCompositeExtract %v2float %84 0
%89 = OpFOrdEqual %v2bool %88 %46
%90 = OpAll %bool %89
%91 = OpCompositeExtract %v2float %84 1
%92 = OpFOrdEqual %v2bool %91 %86
%93 = OpAll %bool %92
%94 = OpLogicalAnd %bool %90 %93
OpBranch %83
%83 = OpLabel
%95 = OpPhi %bool %false %28 %94 %82
OpSelectionMerge %97 None
OpBranchConditional %95 %96 %97
%96 = OpLabel
%98 = OpLoad %mat3v3float %h33
%110 = OpCompositeExtract %v3float %98 0
%111 = OpFOrdEqual %v3bool %110 %105
%112 = OpAll %bool %111
%113 = OpCompositeExtract %v3float %98 1
%114 = OpFOrdEqual %v3bool %113 %106
%115 = OpAll %bool %114
%116 = OpLogicalAnd %bool %112 %115
%117 = OpCompositeExtract %v3float %98 2
%118 = OpFOrdEqual %v3bool %117 %107
%119 = OpAll %bool %118
%120 = OpLogicalAnd %bool %116 %119
OpBranch %97
%97 = OpLabel
%121 = OpPhi %bool %false %83 %120 %96
OpSelectionMerge %126 None
OpBranchConditional %121 %124 %125
%124 = OpLabel
%127 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%130 = OpLoad %v4float %127
OpStore %122 %130
OpBranch %126
%125 = OpLabel
%131 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%133 = OpLoad %v4float %131
OpStore %122 %133
OpBranch %126
%126 = OpLabel
%134 = OpLoad %v4float %122
OpReturnValue %134
OpFunctionEnd
