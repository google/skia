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
OpName %_2_m24 "_2_m24"
OpName %_3_m32 "_3_m32"
OpName %_4_m34 "_4_m34"
OpName %_5_m42 "_5_m42"
OpName %_6_m43 "_6_m43"
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
OpDecorate %38 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
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
%false = OpConstantFalse %bool
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_v3float = OpTypePointer Function %v3float
%mat2v4float = OpTypeMatrix %v4float 2
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
%float_3 = OpConstant %float 3
%_ptr_Function_v4float = OpTypePointer Function %v4float
%mat3v2float = OpTypeMatrix %v2float 3
%_ptr_Function_mat3v2float = OpTypePointer Function %mat3v2float
%float_4 = OpConstant %float 4
%mat3v4float = OpTypeMatrix %v4float 3
%_ptr_Function_mat3v4float = OpTypePointer Function %mat3v4float
%float_5 = OpConstant %float 5
%mat4v2float = OpTypeMatrix %v2float 4
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
%float_6 = OpConstant %float 6
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
%float_7 = OpConstant %float 7
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
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
%_2_m24 = OpVariable %_ptr_Function_mat2v4float Function
%_3_m32 = OpVariable %_ptr_Function_mat3v2float Function
%_4_m34 = OpVariable %_ptr_Function_mat3v4float Function
%_5_m42 = OpVariable %_ptr_Function_mat4v2float Function
%_6_m43 = OpVariable %_ptr_Function_mat4v3float Function
%132 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%35 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%36 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%34 = OpCompositeConstruct %mat2v3float %35 %36
OpStore %_1_m23 %34
%38 = OpLoad %bool %_0_ok
OpSelectionMerge %40 None
OpBranchConditional %38 %39 %40
%39 = OpLabel
%43 = OpAccessChain %_ptr_Function_v3float %_1_m23 %int_0
%45 = OpLoad %v3float %43
%46 = OpCompositeExtract %float %45 0
%47 = OpFOrdEqual %bool %46 %float_2
OpBranch %40
%40 = OpLabel
%48 = OpPhi %bool %false %25 %47 %39
OpStore %_0_ok %48
%54 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%55 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%53 = OpCompositeConstruct %mat2v4float %54 %55
OpStore %_2_m24 %53
%56 = OpLoad %bool %_0_ok
OpSelectionMerge %58 None
OpBranchConditional %56 %57 %58
%57 = OpLabel
%59 = OpAccessChain %_ptr_Function_v4float %_2_m24 %int_0
%61 = OpLoad %v4float %59
%62 = OpCompositeExtract %float %61 0
%63 = OpFOrdEqual %bool %62 %float_3
OpBranch %58
%58 = OpLabel
%64 = OpPhi %bool %false %40 %63 %57
OpStore %_0_ok %64
%70 = OpCompositeConstruct %v2float %float_4 %float_0
%71 = OpCompositeConstruct %v2float %float_0 %float_4
%72 = OpCompositeConstruct %v2float %float_0 %float_0
%69 = OpCompositeConstruct %mat3v2float %70 %71 %72
OpStore %_3_m32 %69
%73 = OpLoad %bool %_0_ok
OpSelectionMerge %75 None
OpBranchConditional %73 %74 %75
%74 = OpLabel
%76 = OpAccessChain %_ptr_Function_v2float %_3_m32 %int_0
%77 = OpLoad %v2float %76
%78 = OpCompositeExtract %float %77 0
%79 = OpFOrdEqual %bool %78 %float_4
OpBranch %75
%75 = OpLabel
%80 = OpPhi %bool %false %58 %79 %74
OpStore %_0_ok %80
%86 = OpCompositeConstruct %v4float %float_5 %float_0 %float_0 %float_0
%87 = OpCompositeConstruct %v4float %float_0 %float_5 %float_0 %float_0
%88 = OpCompositeConstruct %v4float %float_0 %float_0 %float_5 %float_0
%85 = OpCompositeConstruct %mat3v4float %86 %87 %88
OpStore %_4_m34 %85
%89 = OpLoad %bool %_0_ok
OpSelectionMerge %91 None
OpBranchConditional %89 %90 %91
%90 = OpLabel
%92 = OpAccessChain %_ptr_Function_v4float %_4_m34 %int_0
%93 = OpLoad %v4float %92
%94 = OpCompositeExtract %float %93 0
%95 = OpFOrdEqual %bool %94 %float_5
OpBranch %91
%91 = OpLabel
%96 = OpPhi %bool %false %75 %95 %90
OpStore %_0_ok %96
%102 = OpCompositeConstruct %v2float %float_6 %float_0
%103 = OpCompositeConstruct %v2float %float_0 %float_6
%104 = OpCompositeConstruct %v2float %float_0 %float_0
%105 = OpCompositeConstruct %v2float %float_0 %float_0
%101 = OpCompositeConstruct %mat4v2float %102 %103 %104 %105
OpStore %_5_m42 %101
%106 = OpLoad %bool %_0_ok
OpSelectionMerge %108 None
OpBranchConditional %106 %107 %108
%107 = OpLabel
%109 = OpAccessChain %_ptr_Function_v2float %_5_m42 %int_0
%110 = OpLoad %v2float %109
%111 = OpCompositeExtract %float %110 0
%112 = OpFOrdEqual %bool %111 %float_6
OpBranch %108
%108 = OpLabel
%113 = OpPhi %bool %false %91 %112 %107
OpStore %_0_ok %113
%119 = OpCompositeConstruct %v3float %float_7 %float_0 %float_0
%120 = OpCompositeConstruct %v3float %float_0 %float_7 %float_0
%121 = OpCompositeConstruct %v3float %float_0 %float_0 %float_7
%122 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%118 = OpCompositeConstruct %mat4v3float %119 %120 %121 %122
OpStore %_6_m43 %118
%123 = OpLoad %bool %_0_ok
OpSelectionMerge %125 None
OpBranchConditional %123 %124 %125
%124 = OpLabel
%126 = OpAccessChain %_ptr_Function_v3float %_6_m43 %int_0
%127 = OpLoad %v3float %126
%128 = OpCompositeExtract %float %127 0
%129 = OpFOrdEqual %bool %128 %float_7
OpBranch %125
%125 = OpLabel
%130 = OpPhi %bool %false %108 %129 %124
OpStore %_0_ok %130
%131 = OpLoad %bool %_0_ok
OpSelectionMerge %135 None
OpBranchConditional %131 %133 %134
%133 = OpLabel
%136 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%138 = OpLoad %v4float %136
OpStore %132 %138
OpBranch %135
%134 = OpLabel
%139 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%141 = OpLoad %v4float %139
OpStore %132 %141
OpBranch %135
%135 = OpLabel
%142 = OpLoad %v4float %132
OpReturnValue %142
OpFunctionEnd
