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
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
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
%v2bool = OpTypeVector %bool 2
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
%int_3 = OpConstant %int 3
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%float_7 = OpConstant %float 7
%float_8 = OpConstant %float 8
%float_9 = OpConstant %float 9
%v3bool = OpTypeVector %bool 3
%float_100 = OpConstant %float 100
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
%135 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%33 = OpLoad %bool %_0_ok
OpSelectionMerge %35 None
OpBranchConditional %33 %34 %35
%34 = OpLabel
%36 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%40 = OpLoad %mat2v2float %36
%45 = OpCompositeConstruct %v2float %float_1 %float_2
%46 = OpCompositeConstruct %v2float %float_3 %float_4
%47 = OpCompositeConstruct %mat2v2float %45 %46
%49 = OpCompositeExtract %v2float %40 0
%50 = OpCompositeExtract %v2float %47 0
%51 = OpFOrdEqual %v2bool %49 %50
%52 = OpAll %bool %51
%53 = OpCompositeExtract %v2float %40 1
%54 = OpCompositeExtract %v2float %47 1
%55 = OpFOrdEqual %v2bool %53 %54
%56 = OpAll %bool %55
%57 = OpLogicalAnd %bool %52 %56
OpBranch %35
%35 = OpLabel
%58 = OpPhi %bool %false %28 %57 %34
OpStore %_0_ok %58
%59 = OpLoad %bool %_0_ok
OpSelectionMerge %61 None
OpBranchConditional %59 %60 %61
%60 = OpLabel
%62 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%65 = OpLoad %mat3v3float %62
%71 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%72 = OpCompositeConstruct %v3float %float_4 %float_5 %float_6
%73 = OpCompositeConstruct %v3float %float_7 %float_8 %float_9
%74 = OpCompositeConstruct %mat3v3float %71 %72 %73
%76 = OpCompositeExtract %v3float %65 0
%77 = OpCompositeExtract %v3float %74 0
%78 = OpFOrdEqual %v3bool %76 %77
%79 = OpAll %bool %78
%80 = OpCompositeExtract %v3float %65 1
%81 = OpCompositeExtract %v3float %74 1
%82 = OpFOrdEqual %v3bool %80 %81
%83 = OpAll %bool %82
%84 = OpLogicalAnd %bool %79 %83
%85 = OpCompositeExtract %v3float %65 2
%86 = OpCompositeExtract %v3float %74 2
%87 = OpFOrdEqual %v3bool %85 %86
%88 = OpAll %bool %87
%89 = OpLogicalAnd %bool %84 %88
OpBranch %61
%61 = OpLabel
%90 = OpPhi %bool %false %35 %89 %60
OpStore %_0_ok %90
%91 = OpLoad %bool %_0_ok
OpSelectionMerge %93 None
OpBranchConditional %91 %92 %93
%92 = OpLabel
%94 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%95 = OpLoad %mat2v2float %94
%98 = OpCompositeConstruct %v2float %float_100 %float_0
%99 = OpCompositeConstruct %v2float %float_0 %float_100
%97 = OpCompositeConstruct %mat2v2float %98 %99
%100 = OpCompositeExtract %v2float %95 0
%101 = OpCompositeExtract %v2float %97 0
%102 = OpFOrdNotEqual %v2bool %100 %101
%103 = OpAny %bool %102
%104 = OpCompositeExtract %v2float %95 1
%105 = OpCompositeExtract %v2float %97 1
%106 = OpFOrdNotEqual %v2bool %104 %105
%107 = OpAny %bool %106
%108 = OpLogicalOr %bool %103 %107
OpBranch %93
%93 = OpLabel
%109 = OpPhi %bool %false %61 %108 %92
OpStore %_0_ok %109
%110 = OpLoad %bool %_0_ok
OpSelectionMerge %112 None
OpBranchConditional %110 %111 %112
%111 = OpLabel
%113 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%114 = OpLoad %mat3v3float %113
%115 = OpCompositeConstruct %v3float %float_9 %float_8 %float_7
%116 = OpCompositeConstruct %v3float %float_6 %float_5 %float_4
%117 = OpCompositeConstruct %v3float %float_3 %float_2 %float_1
%118 = OpCompositeConstruct %mat3v3float %115 %116 %117
%119 = OpCompositeExtract %v3float %114 0
%120 = OpCompositeExtract %v3float %118 0
%121 = OpFOrdNotEqual %v3bool %119 %120
%122 = OpAny %bool %121
%123 = OpCompositeExtract %v3float %114 1
%124 = OpCompositeExtract %v3float %118 1
%125 = OpFOrdNotEqual %v3bool %123 %124
%126 = OpAny %bool %125
%127 = OpLogicalOr %bool %122 %126
%128 = OpCompositeExtract %v3float %114 2
%129 = OpCompositeExtract %v3float %118 2
%130 = OpFOrdNotEqual %v3bool %128 %129
%131 = OpAny %bool %130
%132 = OpLogicalOr %bool %127 %131
OpBranch %112
%112 = OpLabel
%133 = OpPhi %bool %false %93 %132 %111
OpStore %_0_ok %133
%134 = OpLoad %bool %_0_ok
OpSelectionMerge %139 None
OpBranchConditional %134 %137 %138
%137 = OpLabel
%140 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%143 = OpLoad %v4float %140
OpStore %135 %143
OpBranch %139
%138 = OpLabel
%144 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%146 = OpLoad %v4float %144
OpStore %135 %146
OpBranch %139
%139 = OpLabel
%147 = OpLoad %v4float %135
OpReturnValue %147
OpFunctionEnd
