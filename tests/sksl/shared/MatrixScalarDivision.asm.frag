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
OpName %ok "ok"
OpName %matrixScalarDivide "matrixScalarDivide"
OpName %scalarMatrixDivide "scalarMatrixDivide"
OpName %delta "delta"
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
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 3 Offset 64
OpMemberDecorate %_UniformBuffer 3 ColMajor
OpMemberDecorate %_UniformBuffer 3 MatrixStride 16
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %matrixScalarDivide RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %scalarMatrixDivide RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %delta RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
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
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float %mat3v3float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%19 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%26 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
%int = OpTypeInt 32 1
%int_3 = OpConstant %int 3
%float_0_5 = OpConstant %float 0.5
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_12 = OpConstant %float 12
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int_2 = OpConstant %int 2
%false = OpConstantFalse %bool
%float_1 = OpConstant %float 1
%float_1_5 = OpConstant %float 1.5
%float_2 = OpConstant %float 2
%float_2_5 = OpConstant %float 2.5
%float_3 = OpConstant %float 3
%float_3_5 = OpConstant %float 3.5
%float_4 = OpConstant %float 4
%float_4_5 = OpConstant %float 4.5
%v3bool = OpTypeVector %bool 3
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%float_6 = OpConstant %float 6
%96 = OpConstantComposite %v4float %float_12 %float_6 %float_4 %float_3
%float_0_0500000007 = OpConstant %float 0.0500000007
%106 = OpConstantComposite %v4float %float_0_0500000007 %float_0_0500000007 %float_0_0500000007 %float_0_0500000007
%v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
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
%ok = OpVariable %_ptr_Function_bool Function
%matrixScalarDivide = OpVariable %_ptr_Function_mat3v3float Function
%scalarMatrixDivide = OpVariable %_ptr_Function_mat2v2float Function
%delta = OpVariable %_ptr_Function_v4float Function
%110 = OpVariable %_ptr_Function_v4float Function
OpStore %ok %true
%34 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%38 = OpLoad %mat3v3float %34
%40 = OpMatrixTimesScalar %mat3v3float %38 %float_0_5
OpStore %matrixScalarDivide %40
%44 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%47 = OpLoad %mat2v2float %44
%48 = OpMatrixTimesScalar %mat2v2float %47 %float_12
OpStore %scalarMatrixDivide %48
%50 = OpLoad %bool %ok
OpSelectionMerge %52 None
OpBranchConditional %50 %51 %52
%51 = OpLabel
%53 = OpLoad %mat3v3float %matrixScalarDivide
%63 = OpCompositeConstruct %v3float %float_0_5 %float_1 %float_1_5
%64 = OpCompositeConstruct %v3float %float_2 %float_2_5 %float_3
%65 = OpCompositeConstruct %v3float %float_3_5 %float_4 %float_4_5
%62 = OpCompositeConstruct %mat3v3float %63 %64 %65
%67 = OpCompositeExtract %v3float %53 0
%68 = OpCompositeExtract %v3float %62 0
%69 = OpFOrdEqual %v3bool %67 %68
%70 = OpAll %bool %69
%71 = OpCompositeExtract %v3float %53 1
%72 = OpCompositeExtract %v3float %62 1
%73 = OpFOrdEqual %v3bool %71 %72
%74 = OpAll %bool %73
%75 = OpLogicalAnd %bool %70 %74
%76 = OpCompositeExtract %v3float %53 2
%77 = OpCompositeExtract %v3float %62 2
%78 = OpFOrdEqual %v3bool %76 %77
%79 = OpAll %bool %78
%80 = OpLogicalAnd %bool %75 %79
OpBranch %52
%52 = OpLabel
%81 = OpPhi %bool %false %28 %80 %51
OpStore %ok %81
%85 = OpAccessChain %_ptr_Function_v2float %scalarMatrixDivide %int_0
%86 = OpLoad %v2float %85
%87 = OpCompositeExtract %float %86 0
%88 = OpCompositeExtract %float %86 1
%90 = OpAccessChain %_ptr_Function_v2float %scalarMatrixDivide %int_1
%91 = OpLoad %v2float %90
%92 = OpCompositeExtract %float %91 0
%93 = OpCompositeExtract %float %91 1
%94 = OpCompositeConstruct %v4float %87 %88 %92 %93
%97 = OpFSub %v4float %94 %96
OpStore %delta %97
%98 = OpLoad %bool %ok
OpSelectionMerge %100 None
OpBranchConditional %98 %99 %100
%99 = OpLabel
%104 = OpLoad %v4float %delta
%103 = OpExtInst %v4float %1 FAbs %104
%102 = OpFOrdLessThan %v4bool %103 %106
%101 = OpAll %bool %102
OpBranch %100
%100 = OpLabel
%108 = OpPhi %bool %false %52 %101 %99
OpStore %ok %108
%109 = OpLoad %bool %ok
OpSelectionMerge %113 None
OpBranchConditional %109 %111 %112
%111 = OpLabel
%114 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%116 = OpLoad %v4float %114
OpStore %110 %116
OpBranch %113
%112 = OpLabel
%117 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%118 = OpLoad %v4float %117
OpStore %110 %118
OpBranch %113
%113 = OpLabel
%119 = OpLoad %v4float %110
OpReturnValue %119
OpFunctionEnd
