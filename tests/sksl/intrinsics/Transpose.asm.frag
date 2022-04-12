OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testMatrix2x2"
OpMemberName %_UniformBuffer 1 "testMatrix3x3"
OpMemberName %_UniformBuffer 2 "colorGreen"
OpMemberName %_UniformBuffer 3 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %testMatrix2x3 "testMatrix2x3"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 ColMajor
OpMemberDecorate %_UniformBuffer 0 MatrixStride 16
OpMemberDecorate %_UniformBuffer 1 Offset 32
OpMemberDecorate %_UniformBuffer 1 ColMajor
OpMemberDecorate %_UniformBuffer 1 MatrixStride 16
OpMemberDecorate %_UniformBuffer 2 Offset 80
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 3 Offset 96
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %115 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
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
%_UniformBuffer = OpTypeStruct %mat2v2float %mat3v3float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%19 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%26 = OpTypeFunction %v4float %_ptr_Function_v2float
%mat2v3float = OpTypeMatrix %v3float 2
%_ptr_Function_mat2v3float = OpTypePointer Function %mat2v3float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%38 = OpConstantComposite %v3float %float_1 %float_2 %float_3
%39 = OpConstantComposite %v3float %float_4 %float_5 %float_6
%40 = OpConstantComposite %mat2v3float %38 %39
%false = OpConstantFalse %bool
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%48 = OpConstantComposite %v2float %float_1 %float_3
%49 = OpConstantComposite %v2float %float_2 %float_4
%50 = OpConstantComposite %mat2v2float %48 %49
%v2bool = OpTypeVector %bool 2
%mat3v2float = OpTypeMatrix %v2float 3
%64 = OpConstantComposite %v2float %float_1 %float_4
%65 = OpConstantComposite %v2float %float_2 %float_5
%66 = OpConstantComposite %v2float %float_3 %float_6
%67 = OpConstantComposite %mat3v2float %64 %65 %66
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
%int_1 = OpConstant %int 1
%float_7 = OpConstant %float 7
%float_8 = OpConstant %float 8
%float_9 = OpConstant %float 9
%90 = OpConstantComposite %v3float %float_1 %float_4 %float_7
%91 = OpConstantComposite %v3float %float_2 %float_5 %float_8
%92 = OpConstantComposite %v3float %float_3 %float_6 %float_9
%93 = OpConstantComposite %mat3v3float %90 %91 %92
%v3bool = OpTypeVector %bool 3
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
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
%testMatrix2x3 = OpVariable %_ptr_Function_mat2v3float Function
%107 = OpVariable %_ptr_Function_v4float Function
OpStore %testMatrix2x3 %40
%43 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_0
%47 = OpLoad %mat2v2float %43
%42 = OpTranspose %mat2v2float %47
%52 = OpCompositeExtract %v2float %42 0
%53 = OpFOrdEqual %v2bool %52 %48
%54 = OpAll %bool %53
%55 = OpCompositeExtract %v2float %42 1
%56 = OpFOrdEqual %v2bool %55 %49
%57 = OpAll %bool %56
%58 = OpLogicalAnd %bool %54 %57
OpSelectionMerge %60 None
OpBranchConditional %58 %59 %60
%59 = OpLabel
%62 = OpLoad %mat2v3float %testMatrix2x3
%61 = OpTranspose %mat3v2float %62
%68 = OpCompositeExtract %v2float %61 0
%69 = OpFOrdEqual %v2bool %68 %64
%70 = OpAll %bool %69
%71 = OpCompositeExtract %v2float %61 1
%72 = OpFOrdEqual %v2bool %71 %65
%73 = OpAll %bool %72
%74 = OpLogicalAnd %bool %70 %73
%75 = OpCompositeExtract %v2float %61 2
%76 = OpFOrdEqual %v2bool %75 %66
%77 = OpAll %bool %76
%78 = OpLogicalAnd %bool %74 %77
OpBranch %60
%60 = OpLabel
%79 = OpPhi %bool %false %28 %78 %59
OpSelectionMerge %81 None
OpBranchConditional %79 %80 %81
%80 = OpLabel
%83 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_1
%86 = OpLoad %mat3v3float %83
%82 = OpTranspose %mat3v3float %86
%95 = OpCompositeExtract %v3float %82 0
%96 = OpFOrdEqual %v3bool %95 %90
%97 = OpAll %bool %96
%98 = OpCompositeExtract %v3float %82 1
%99 = OpFOrdEqual %v3bool %98 %91
%100 = OpAll %bool %99
%101 = OpLogicalAnd %bool %97 %100
%102 = OpCompositeExtract %v3float %82 2
%103 = OpFOrdEqual %v3bool %102 %92
%104 = OpAll %bool %103
%105 = OpLogicalAnd %bool %101 %104
OpBranch %81
%81 = OpLabel
%106 = OpPhi %bool %false %60 %105 %80
OpSelectionMerge %111 None
OpBranchConditional %106 %109 %110
%109 = OpLabel
%112 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%115 = OpLoad %v4float %112
OpStore %107 %115
OpBranch %111
%110 = OpLabel
%116 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%118 = OpLoad %v4float %116
OpStore %107 %118
OpBranch %111
%111 = OpLabel
%119 = OpLoad %v4float %107
OpReturnValue %119
OpFunctionEnd
