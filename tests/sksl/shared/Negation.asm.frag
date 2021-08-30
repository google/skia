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
OpName %test_ivec_b "test_ivec_b"
OpName %one "one"
OpName %two "two"
OpName %ok "ok"
OpName %test_mat_b "test_mat_b"
OpName %ok_0 "ok"
OpName %main "main"
OpName %_0_one "_0_one"
OpName %_1_two "_1_two"
OpName %_4_ok "_4_ok"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %37 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%25 = OpTypeFunction %bool
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%v2int = OpTypeVector %int 2
%v2bool = OpTypeVector %bool 2
%61 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%v3float = OpTypeVector %float 3
%v4bool = OpTypeVector %bool 4
%float_n2 = OpConstant %float -2
%94 = OpConstantComposite %v2float %float_1 %float_n2
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%22 = OpVariable %_ptr_Function_v2float Function
OpStore %22 %21
%24 = OpFunctionCall %v4float %main %22
OpStore %sk_FragColor %24
OpReturn
OpFunctionEnd
%test_ivec_b = OpFunction %bool None %25
%26 = OpLabel
%one = OpVariable %_ptr_Function_int Function
%two = OpVariable %_ptr_Function_int Function
%ok = OpVariable %_ptr_Function_bool Function
OpStore %one %int_1
OpStore %two %int_2
OpStore %ok %true
%37 = OpLoad %bool %ok
OpSelectionMerge %39 None
OpBranchConditional %37 %38 %39
%38 = OpLabel
%43 = OpLoad %int %one
%42 = OpSNegate %int %43
%44 = OpLoad %int %one
%45 = OpLoad %int %one
%46 = OpIAdd %int %44 %45
%47 = OpCompositeConstruct %v2int %42 %46
%40 = OpSNegate %v2int %47
%49 = OpLoad %int %one
%50 = OpLoad %int %two
%51 = OpISub %int %49 %50
%52 = OpCompositeConstruct %v2int %51 %int_2
%48 = OpSNegate %v2int %52
%53 = OpIEqual %v2bool %40 %48
%55 = OpAll %bool %53
OpBranch %39
%39 = OpLabel
%56 = OpPhi %bool %false %26 %55 %38
OpStore %ok %56
%57 = OpLoad %bool %ok
OpReturnValue %57
OpFunctionEnd
%test_mat_b = OpFunction %bool None %25
%58 = OpLabel
%ok_0 = OpVariable %_ptr_Function_bool Function
OpStore %ok_0 %true
%60 = OpLoad %bool %ok_0
OpReturnValue %60
OpFunctionEnd
%main = OpFunction %v4float None %61
%62 = OpFunctionParameter %_ptr_Function_v2float
%63 = OpLabel
%_0_one = OpVariable %_ptr_Function_float Function
%_1_two = OpVariable %_ptr_Function_float Function
%_4_ok = OpVariable %_ptr_Function_bool Function
%113 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_one %float_1
OpStore %_1_two %float_2
OpStore %_4_ok %true
%70 = OpLoad %bool %_4_ok
OpSelectionMerge %72 None
OpBranchConditional %70 %71 %72
%71 = OpLabel
%74 = OpLoad %float %_1_two
%75 = OpCompositeConstruct %v4float %74 %74 %74 %74
%73 = OpFNegate %v4float %75
%77 = OpLoad %float %_1_two
%76 = OpFNegate %float %77
%79 = OpLoad %float %_1_two
%78 = OpFNegate %float %79
%80 = OpCompositeConstruct %v3float %78 %78 %78
%82 = OpCompositeExtract %float %80 0
%83 = OpCompositeExtract %float %80 1
%84 = OpCompositeExtract %float %80 2
%85 = OpCompositeConstruct %v4float %76 %82 %83 %84
%86 = OpFOrdEqual %v4bool %73 %85
%88 = OpAll %bool %86
OpBranch %72
%72 = OpLabel
%89 = OpPhi %bool %false %63 %88 %71
OpStore %_4_ok %89
%90 = OpLoad %bool %_4_ok
OpSelectionMerge %92 None
OpBranchConditional %90 %91 %92
%91 = OpLabel
%96 = OpLoad %float %_0_one
%97 = OpLoad %float %_1_two
%98 = OpFSub %float %96 %97
%99 = OpLoad %float %_1_two
%100 = OpCompositeConstruct %v2float %98 %99
%95 = OpFNegate %v2float %100
%101 = OpFOrdEqual %v2bool %94 %95
%102 = OpAll %bool %101
OpBranch %92
%92 = OpLabel
%103 = OpPhi %bool %false %72 %102 %91
OpStore %_4_ok %103
%104 = OpLoad %bool %_4_ok
OpSelectionMerge %106 None
OpBranchConditional %104 %105 %106
%105 = OpLabel
%107 = OpFunctionCall %bool %test_ivec_b
OpBranch %106
%106 = OpLabel
%108 = OpPhi %bool %false %92 %107 %105
OpSelectionMerge %110 None
OpBranchConditional %108 %109 %110
%109 = OpLabel
%111 = OpFunctionCall %bool %test_mat_b
OpBranch %110
%110 = OpLabel
%112 = OpPhi %bool %false %106 %111 %109
OpSelectionMerge %117 None
OpBranchConditional %112 %115 %116
%115 = OpLabel
%118 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%121 = OpLoad %v4float %118
OpStore %113 %121
OpBranch %117
%116 = OpLabel
%122 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%123 = OpLoad %v4float %122
OpStore %113 %123
OpBranch %117
%117 = OpLabel
%124 = OpLoad %v4float %113
OpReturnValue %124
OpFunctionEnd
