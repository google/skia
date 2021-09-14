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
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %h33 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
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
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%float_1 = OpConstant %float 1
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
%int_3 = OpConstant %int 3
%float_2 = OpConstant %float 2
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%float_4 = OpConstant %float 4
%float_6 = OpConstant %float 6
%float_8 = OpConstant %float 8
%float_12 = OpConstant %float 12
%float_14 = OpConstant %float 14
%float_16 = OpConstant %float 16
%float_18 = OpConstant %float 18
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
%138 = OpVariable %_ptr_Function_v4float Function
%34 = OpCompositeConstruct %v2float %float_0 %float_5
%35 = OpCompositeConstruct %v2float %float_10 %float_15
%36 = OpCompositeConstruct %mat2v2float %34 %35
OpStore %h22 %36
%39 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%43 = OpLoad %mat2v2float %39
%46 = OpCompositeConstruct %v2float %float_1 %float_0
%47 = OpCompositeConstruct %v2float %float_0 %float_1
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
%63 = OpCompositeConstruct %v3float %float_2 %float_2 %float_2
%64 = OpCompositeConstruct %v3float %float_2 %float_2 %float_2
%65 = OpCompositeConstruct %v3float %float_2 %float_2 %float_2
%66 = OpCompositeConstruct %mat3v3float %63 %64 %65
%67 = OpCompositeExtract %v3float %61 0
%68 = OpCompositeExtract %v3float %66 0
%69 = OpFMul %v3float %67 %68
%70 = OpCompositeExtract %v3float %61 1
%71 = OpCompositeExtract %v3float %66 1
%72 = OpFMul %v3float %70 %71
%73 = OpCompositeExtract %v3float %61 2
%74 = OpCompositeExtract %v3float %66 2
%75 = OpFMul %v3float %73 %74
%76 = OpCompositeConstruct %mat3v3float %69 %72 %75
OpStore %h33 %76
%78 = OpLoad %mat2v2float %h22
%79 = OpCompositeConstruct %v2float %float_0 %float_5
%80 = OpCompositeConstruct %v2float %float_10 %float_15
%81 = OpCompositeConstruct %mat2v2float %79 %80
%83 = OpCompositeExtract %v2float %78 0
%84 = OpCompositeExtract %v2float %81 0
%85 = OpFOrdEqual %v2bool %83 %84
%86 = OpAll %bool %85
%87 = OpCompositeExtract %v2float %78 1
%88 = OpCompositeExtract %v2float %81 1
%89 = OpFOrdEqual %v2bool %87 %88
%90 = OpAll %bool %89
%91 = OpLogicalAnd %bool %86 %90
OpSelectionMerge %93 None
OpBranchConditional %91 %92 %93
%92 = OpLabel
%94 = OpLoad %mat2v2float %f22
%96 = OpCompositeConstruct %v2float %float_1 %float_0
%97 = OpCompositeConstruct %v2float %float_0 %float_4
%98 = OpCompositeConstruct %mat2v2float %96 %97
%99 = OpCompositeExtract %v2float %94 0
%100 = OpCompositeExtract %v2float %98 0
%101 = OpFOrdEqual %v2bool %99 %100
%102 = OpAll %bool %101
%103 = OpCompositeExtract %v2float %94 1
%104 = OpCompositeExtract %v2float %98 1
%105 = OpFOrdEqual %v2bool %103 %104
%106 = OpAll %bool %105
%107 = OpLogicalAnd %bool %102 %106
OpBranch %93
%93 = OpLabel
%108 = OpPhi %bool %false %28 %107 %92
OpSelectionMerge %110 None
OpBranchConditional %108 %109 %110
%109 = OpLabel
%111 = OpLoad %mat3v3float %h33
%118 = OpCompositeConstruct %v3float %float_2 %float_4 %float_6
%119 = OpCompositeConstruct %v3float %float_8 %float_10 %float_12
%120 = OpCompositeConstruct %v3float %float_14 %float_16 %float_18
%121 = OpCompositeConstruct %mat3v3float %118 %119 %120
%123 = OpCompositeExtract %v3float %111 0
%124 = OpCompositeExtract %v3float %121 0
%125 = OpFOrdEqual %v3bool %123 %124
%126 = OpAll %bool %125
%127 = OpCompositeExtract %v3float %111 1
%128 = OpCompositeExtract %v3float %121 1
%129 = OpFOrdEqual %v3bool %127 %128
%130 = OpAll %bool %129
%131 = OpLogicalAnd %bool %126 %130
%132 = OpCompositeExtract %v3float %111 2
%133 = OpCompositeExtract %v3float %121 2
%134 = OpFOrdEqual %v3bool %132 %133
%135 = OpAll %bool %134
%136 = OpLogicalAnd %bool %131 %135
OpBranch %110
%110 = OpLabel
%137 = OpPhi %bool %false %93 %136 %109
OpSelectionMerge %142 None
OpBranchConditional %137 %140 %141
%140 = OpLabel
%143 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%146 = OpLoad %v4float %143
OpStore %138 %146
OpBranch %142
%141 = OpLabel
%147 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%149 = OpLoad %v4float %147
OpStore %138 %149
OpBranch %142
%142 = OpLabel
%150 = OpLoad %v4float %138
OpReturnValue %150
OpFunctionEnd
