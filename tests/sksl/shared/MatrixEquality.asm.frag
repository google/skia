OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpMemberName %_UniformBuffer 2 "testMatrix2x2"
OpMemberName %_UniformBuffer 3 "testMatrix3x3"
OpName %_entrypoint_v "_entrypoint_v"
OpName %test_equality_b "test_equality_b"
OpName %ok "ok"
OpName %main "main"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
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
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %39 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float %mat3v3float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%20 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%23 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%27 = OpTypeFunction %bool
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%44 = OpConstantComposite %v2float %float_1 %float_2
%45 = OpConstantComposite %v2float %float_3 %float_4
%46 = OpConstantComposite %mat2v2float %44 %45
%v2bool = OpTypeVector %bool 2
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
%int_3 = OpConstant %int 3
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%float_7 = OpConstant %float 7
%float_8 = OpConstant %float 8
%float_9 = OpConstant %float 9
%67 = OpConstantComposite %v3float %float_1 %float_2 %float_3
%68 = OpConstantComposite %v3float %float_4 %float_5 %float_6
%69 = OpConstantComposite %v3float %float_7 %float_8 %float_9
%70 = OpConstantComposite %mat3v3float %67 %68 %69
%v3bool = OpTypeVector %bool 3
%float_100 = OpConstant %float 100
%89 = OpConstantComposite %v2float %float_100 %float_0
%90 = OpConstantComposite %v2float %float_0 %float_100
%91 = OpConstantComposite %mat2v2float %89 %90
%104 = OpConstantComposite %v3float %float_9 %float_8 %float_7
%105 = OpConstantComposite %v3float %float_6 %float_5 %float_4
%106 = OpConstantComposite %v3float %float_3 %float_2 %float_1
%107 = OpConstantComposite %mat3v3float %104 %105 %106
%120 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %20
%21 = OpLabel
%24 = OpVariable %_ptr_Function_v2float Function
OpStore %24 %23
%26 = OpFunctionCall %v4float %main %24
OpStore %sk_FragColor %26
OpReturn
OpFunctionEnd
%test_equality_b = OpFunction %bool None %27
%28 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
OpStore %ok %true
OpSelectionMerge %34 None
OpBranchConditional %true %33 %34
%33 = OpLabel
%35 = OpAccessChain %_ptr_Uniform_mat2v2float %11 %int_2
%39 = OpLoad %mat2v2float %35
%48 = OpCompositeExtract %v2float %39 0
%49 = OpFOrdEqual %v2bool %48 %44
%50 = OpAll %bool %49
%51 = OpCompositeExtract %v2float %39 1
%52 = OpFOrdEqual %v2bool %51 %45
%53 = OpAll %bool %52
%54 = OpLogicalAnd %bool %50 %53
OpBranch %34
%34 = OpLabel
%55 = OpPhi %bool %false %28 %54 %33
OpStore %ok %55
OpSelectionMerge %57 None
OpBranchConditional %55 %56 %57
%56 = OpLabel
%58 = OpAccessChain %_ptr_Uniform_mat3v3float %11 %int_3
%61 = OpLoad %mat3v3float %58
%72 = OpCompositeExtract %v3float %61 0
%73 = OpFOrdEqual %v3bool %72 %67
%74 = OpAll %bool %73
%75 = OpCompositeExtract %v3float %61 1
%76 = OpFOrdEqual %v3bool %75 %68
%77 = OpAll %bool %76
%78 = OpLogicalAnd %bool %74 %77
%79 = OpCompositeExtract %v3float %61 2
%80 = OpFOrdEqual %v3bool %79 %69
%81 = OpAll %bool %80
%82 = OpLogicalAnd %bool %78 %81
OpBranch %57
%57 = OpLabel
%83 = OpPhi %bool %false %34 %82 %56
OpStore %ok %83
OpSelectionMerge %85 None
OpBranchConditional %83 %84 %85
%84 = OpLabel
%86 = OpAccessChain %_ptr_Uniform_mat2v2float %11 %int_2
%87 = OpLoad %mat2v2float %86
%92 = OpCompositeExtract %v2float %87 0
%93 = OpFUnordNotEqual %v2bool %92 %89
%94 = OpAny %bool %93
%95 = OpCompositeExtract %v2float %87 1
%96 = OpFUnordNotEqual %v2bool %95 %90
%97 = OpAny %bool %96
%98 = OpLogicalOr %bool %94 %97
OpBranch %85
%85 = OpLabel
%99 = OpPhi %bool %false %57 %98 %84
OpStore %ok %99
OpSelectionMerge %101 None
OpBranchConditional %99 %100 %101
%100 = OpLabel
%102 = OpAccessChain %_ptr_Uniform_mat3v3float %11 %int_3
%103 = OpLoad %mat3v3float %102
%108 = OpCompositeExtract %v3float %103 0
%109 = OpFUnordNotEqual %v3bool %108 %104
%110 = OpAny %bool %109
%111 = OpCompositeExtract %v3float %103 1
%112 = OpFUnordNotEqual %v3bool %111 %105
%113 = OpAny %bool %112
%114 = OpLogicalOr %bool %110 %113
%115 = OpCompositeExtract %v3float %103 2
%116 = OpFUnordNotEqual %v3bool %115 %106
%117 = OpAny %bool %116
%118 = OpLogicalOr %bool %114 %117
OpBranch %101
%101 = OpLabel
%119 = OpPhi %bool %false %85 %118 %100
OpStore %ok %119
OpReturnValue %119
OpFunctionEnd
%main = OpFunction %v4float None %120
%121 = OpFunctionParameter %_ptr_Function_v2float
%122 = OpLabel
%124 = OpVariable %_ptr_Function_v4float Function
%123 = OpFunctionCall %bool %test_equality_b
OpSelectionMerge %128 None
OpBranchConditional %123 %126 %127
%126 = OpLabel
%129 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%132 = OpLoad %v4float %129
OpStore %124 %132
OpBranch %128
%127 = OpLabel
%133 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%135 = OpLoad %v4float %133
OpStore %124 %135
OpBranch %128
%128 = OpLabel
%136 = OpLoad %v4float %124
OpReturnValue %136
OpFunctionEnd
