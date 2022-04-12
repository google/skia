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
OpName %_0_ok "_0_ok"
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
OpDecorate %33 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
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
%false = OpConstantFalse %bool
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%45 = OpConstantComposite %v2float %float_1 %float_2
%46 = OpConstantComposite %v2float %float_3 %float_4
%47 = OpConstantComposite %mat2v2float %45 %46
%v2bool = OpTypeVector %bool 2
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
%int_3 = OpConstant %int 3
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%float_7 = OpConstant %float 7
%float_8 = OpConstant %float 8
%float_9 = OpConstant %float 9
%69 = OpConstantComposite %v3float %float_1 %float_2 %float_3
%70 = OpConstantComposite %v3float %float_4 %float_5 %float_6
%71 = OpConstantComposite %v3float %float_7 %float_8 %float_9
%72 = OpConstantComposite %mat3v3float %69 %70 %71
%v3bool = OpTypeVector %bool 3
%float_100 = OpConstant %float 100
%93 = OpConstantComposite %v2float %float_100 %float_0
%94 = OpConstantComposite %v2float %float_0 %float_100
%110 = OpConstantComposite %v3float %float_9 %float_8 %float_7
%111 = OpConstantComposite %v3float %float_6 %float_5 %float_4
%112 = OpConstantComposite %v3float %float_3 %float_2 %float_1
%113 = OpConstantComposite %mat3v3float %110 %111 %112
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
%_0_ok = OpVariable %_ptr_Function_bool Function
%127 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%33 = OpLoad %bool %_0_ok
OpSelectionMerge %35 None
OpBranchConditional %33 %34 %35
%34 = OpLabel
%36 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%40 = OpLoad %mat2v2float %36
%49 = OpCompositeExtract %v2float %40 0
%50 = OpFOrdEqual %v2bool %49 %45
%51 = OpAll %bool %50
%52 = OpCompositeExtract %v2float %40 1
%53 = OpFOrdEqual %v2bool %52 %46
%54 = OpAll %bool %53
%55 = OpLogicalAnd %bool %51 %54
OpBranch %35
%35 = OpLabel
%56 = OpPhi %bool %false %28 %55 %34
OpStore %_0_ok %56
%57 = OpLoad %bool %_0_ok
OpSelectionMerge %59 None
OpBranchConditional %57 %58 %59
%58 = OpLabel
%60 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%63 = OpLoad %mat3v3float %60
%74 = OpCompositeExtract %v3float %63 0
%75 = OpFOrdEqual %v3bool %74 %69
%76 = OpAll %bool %75
%77 = OpCompositeExtract %v3float %63 1
%78 = OpFOrdEqual %v3bool %77 %70
%79 = OpAll %bool %78
%80 = OpLogicalAnd %bool %76 %79
%81 = OpCompositeExtract %v3float %63 2
%82 = OpFOrdEqual %v3bool %81 %71
%83 = OpAll %bool %82
%84 = OpLogicalAnd %bool %80 %83
OpBranch %59
%59 = OpLabel
%85 = OpPhi %bool %false %35 %84 %58
OpStore %_0_ok %85
%86 = OpLoad %bool %_0_ok
OpSelectionMerge %88 None
OpBranchConditional %86 %87 %88
%87 = OpLabel
%89 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%90 = OpLoad %mat2v2float %89
%92 = OpCompositeConstruct %mat2v2float %93 %94
%95 = OpCompositeExtract %v2float %90 0
%96 = OpCompositeExtract %v2float %92 0
%97 = OpFUnordNotEqual %v2bool %95 %96
%98 = OpAny %bool %97
%99 = OpCompositeExtract %v2float %90 1
%100 = OpCompositeExtract %v2float %92 1
%101 = OpFUnordNotEqual %v2bool %99 %100
%102 = OpAny %bool %101
%103 = OpLogicalOr %bool %98 %102
OpBranch %88
%88 = OpLabel
%104 = OpPhi %bool %false %59 %103 %87
OpStore %_0_ok %104
%105 = OpLoad %bool %_0_ok
OpSelectionMerge %107 None
OpBranchConditional %105 %106 %107
%106 = OpLabel
%108 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%109 = OpLoad %mat3v3float %108
%114 = OpCompositeExtract %v3float %109 0
%115 = OpFUnordNotEqual %v3bool %114 %110
%116 = OpAny %bool %115
%117 = OpCompositeExtract %v3float %109 1
%118 = OpFUnordNotEqual %v3bool %117 %111
%119 = OpAny %bool %118
%120 = OpLogicalOr %bool %116 %119
%121 = OpCompositeExtract %v3float %109 2
%122 = OpFUnordNotEqual %v3bool %121 %112
%123 = OpAny %bool %122
%124 = OpLogicalOr %bool %120 %123
OpBranch %107
%107 = OpLabel
%125 = OpPhi %bool %false %88 %124 %106
OpStore %_0_ok %125
%126 = OpLoad %bool %_0_ok
OpSelectionMerge %131 None
OpBranchConditional %126 %129 %130
%129 = OpLabel
%132 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%135 = OpLoad %v4float %132
OpStore %127 %135
OpBranch %131
%130 = OpLabel
%136 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%138 = OpLoad %v4float %136
OpStore %127 %138
OpBranch %131
%131 = OpLabel
%139 = OpLoad %v4float %127
OpReturnValue %139
OpFunctionEnd
