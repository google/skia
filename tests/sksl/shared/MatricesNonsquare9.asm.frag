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
OpName %_0_ok "_0_ok"
OpName %_1_m23 "_1_m23"
OpName %_3_m32 "_3_m32"
OpName %_4_m34 "_4_m34"
OpName %_6_m43 "_6_m43"
OpName %_7_m22 "_7_m22"
OpName %_8_m33 "_8_m33"
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
OpDecorate %69 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%v3float = OpTypeVector %float 3
%mat2v3float = OpTypeMatrix %v3float 2
%_ptr_Function_mat2v3float = OpTypePointer Function %mat2v3float
%float_2 = OpConstant %float 2
%mat3v2float = OpTypeMatrix %v2float 3
%_ptr_Function_mat3v2float = OpTypePointer Function %mat3v2float
%float_4 = OpConstant %float 4
%mat3v4float = OpTypeMatrix %v4float 3
%_ptr_Function_mat3v4float = OpTypePointer Function %mat3v4float
%float_5 = OpConstant %float 5
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
%float_7 = OpConstant %float 7
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%false = OpConstantFalse %bool
%float_8 = OpConstant %float 8
%v2bool = OpTypeVector %bool 2
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%float_35 = OpConstant %float 35
%v3bool = OpTypeVector %bool 3
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
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m23 = OpVariable %_ptr_Function_mat2v3float Function
%_3_m32 = OpVariable %_ptr_Function_mat3v2float Function
%_4_m34 = OpVariable %_ptr_Function_mat3v4float Function
%_6_m43 = OpVariable %_ptr_Function_mat4v3float Function
%_7_m22 = OpVariable %_ptr_Function_mat2v2float Function
%_8_m33 = OpVariable %_ptr_Function_mat3v3float Function
%120 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%35 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%36 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%34 = OpCompositeConstruct %mat2v3float %35 %36
OpStore %_1_m23 %34
%42 = OpCompositeConstruct %v2float %float_4 %float_0
%43 = OpCompositeConstruct %v2float %float_0 %float_4
%44 = OpCompositeConstruct %v2float %float_0 %float_0
%41 = OpCompositeConstruct %mat3v2float %42 %43 %44
OpStore %_3_m32 %41
%50 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%51 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%52 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%49 = OpCompositeConstruct %mat3v4float %50 %51 %52
OpStore %_4_m34 %49
%58 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%59 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%60 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%61 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%57 = OpCompositeConstruct %mat4v3float %58 %59 %60 %61
OpStore %_6_m43 %57
%65 = OpLoad %mat3v2float %_3_m32
%66 = OpLoad %mat2v3float %_1_m23
%67 = OpMatrixTimesMatrix %mat2v2float %65 %66
OpStore %_7_m22 %67
%69 = OpLoad %bool %_0_ok
OpSelectionMerge %71 None
OpBranchConditional %69 %70 %71
%70 = OpLabel
%72 = OpLoad %mat2v2float %_7_m22
%75 = OpCompositeConstruct %v2float %float_8 %float_0
%76 = OpCompositeConstruct %v2float %float_0 %float_8
%74 = OpCompositeConstruct %mat2v2float %75 %76
%78 = OpCompositeExtract %v2float %72 0
%79 = OpCompositeExtract %v2float %74 0
%80 = OpFOrdEqual %v2bool %78 %79
%81 = OpAll %bool %80
%82 = OpCompositeExtract %v2float %72 1
%83 = OpCompositeExtract %v2float %74 1
%84 = OpFOrdEqual %v2bool %82 %83
%85 = OpAll %bool %84
%86 = OpLogicalAnd %bool %81 %85
OpBranch %71
%71 = OpLabel
%87 = OpPhi %bool %false %25 %86 %70
OpStore %_0_ok %87
%91 = OpLoad %mat4v3float %_6_m43
%92 = OpLoad %mat3v4float %_4_m34
%93 = OpMatrixTimesMatrix %mat3v3float %91 %92
OpStore %_8_m33 %93
%94 = OpLoad %bool %_0_ok
OpSelectionMerge %96 None
OpBranchConditional %94 %95 %96
%95 = OpLabel
%97 = OpLoad %mat3v3float %_8_m33
%100 = OpCompositeConstruct %v3float %float_35 %float_0 %float_0
%101 = OpCompositeConstruct %v3float %float_0 %float_35 %float_0
%102 = OpCompositeConstruct %v3float %float_0 %float_0 %float_35
%99 = OpCompositeConstruct %mat3v3float %100 %101 %102
%104 = OpCompositeExtract %v3float %97 0
%105 = OpCompositeExtract %v3float %99 0
%106 = OpFOrdEqual %v3bool %104 %105
%107 = OpAll %bool %106
%108 = OpCompositeExtract %v3float %97 1
%109 = OpCompositeExtract %v3float %99 1
%110 = OpFOrdEqual %v3bool %108 %109
%111 = OpAll %bool %110
%112 = OpLogicalAnd %bool %107 %111
%113 = OpCompositeExtract %v3float %97 2
%114 = OpCompositeExtract %v3float %99 2
%115 = OpFOrdEqual %v3bool %113 %114
%116 = OpAll %bool %115
%117 = OpLogicalAnd %bool %112 %116
OpBranch %96
%96 = OpLabel
%118 = OpPhi %bool %false %71 %117 %95
OpStore %_0_ok %118
%119 = OpLoad %bool %_0_ok
OpSelectionMerge %124 None
OpBranchConditional %119 %122 %123
%122 = OpLabel
%125 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%129 = OpLoad %v4float %125
OpStore %120 %129
OpBranch %124
%123 = OpLabel
%130 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%132 = OpLoad %v4float %130
OpStore %120 %132
OpBranch %124
%124 = OpLabel
%133 = OpLoad %v4float %120
OpReturnValue %133
OpFunctionEnd
