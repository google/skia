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
OpName %test_half_b "test_half_b"
OpName %ok "ok"
OpName %v1 "v1"
OpName %v2 "v2"
OpName %main "main"
OpName %_0_ok "_0_ok"
OpName %_1_v1 "_1_v1"
OpName %_2_v2 "_2_v2"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %v1 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %v2 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%16 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %bool
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%float_2 = OpConstant %float 2
%mat3v3float = OpTypeMatrix %v3float 3
%float_3 = OpConstant %float 3
%39 = OpConstantComposite %v3float %float_3 %float_3 %float_3
%false = OpConstantFalse %bool
%float_6 = OpConstant %float 6
%47 = OpConstantComposite %v3float %float_6 %float_6 %float_6
%v3bool = OpTypeVector %bool 3
%float_9 = OpConstant %float 9
%63 = OpConstantComposite %v3float %float_9 %float_9 %float_9
%68 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %16
%17 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%test_half_b = OpFunction %bool None %24
%25 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
%v1 = OpVariable %_ptr_Function_v3float Function
%v2 = OpVariable %_ptr_Function_v3float Function
OpStore %ok %true
%34 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%35 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%36 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%33 = OpCompositeConstruct %mat3v3float %34 %35 %36
%40 = OpMatrixTimesVector %v3float %33 %39
OpStore %v1 %40
%42 = OpLoad %bool %ok
OpSelectionMerge %44 None
OpBranchConditional %42 %43 %44
%43 = OpLabel
%45 = OpLoad %v3float %v1
%48 = OpFOrdEqual %v3bool %45 %47
%50 = OpAll %bool %48
OpBranch %44
%44 = OpLabel
%51 = OpPhi %bool %false %25 %50 %43
OpStore %ok %51
%54 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%55 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%56 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%53 = OpCompositeConstruct %mat3v3float %54 %55 %56
%57 = OpVectorTimesMatrix %v3float %39 %53
OpStore %v2 %57
%58 = OpLoad %bool %ok
OpSelectionMerge %60 None
OpBranchConditional %58 %59 %60
%59 = OpLabel
%61 = OpLoad %v3float %v2
%64 = OpFOrdEqual %v3bool %61 %63
%65 = OpAll %bool %64
OpBranch %60
%60 = OpLabel
%66 = OpPhi %bool %false %44 %65 %59
OpStore %ok %66
%67 = OpLoad %bool %ok
OpReturnValue %67
OpFunctionEnd
%main = OpFunction %v4float None %68
%69 = OpFunctionParameter %_ptr_Function_v2float
%70 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_v1 = OpVariable %_ptr_Function_v3float Function
%_2_v2 = OpVariable %_ptr_Function_v3float Function
%103 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%74 = OpCompositeConstruct %v3float %float_2 %float_0 %float_0
%75 = OpCompositeConstruct %v3float %float_0 %float_2 %float_0
%76 = OpCompositeConstruct %v3float %float_0 %float_0 %float_2
%73 = OpCompositeConstruct %mat3v3float %74 %75 %76
%77 = OpMatrixTimesVector %v3float %73 %39
OpStore %_1_v1 %77
%78 = OpLoad %bool %_0_ok
OpSelectionMerge %80 None
OpBranchConditional %78 %79 %80
%79 = OpLabel
%81 = OpLoad %v3float %_1_v1
%82 = OpFOrdEqual %v3bool %81 %47
%83 = OpAll %bool %82
OpBranch %80
%80 = OpLabel
%84 = OpPhi %bool %false %70 %83 %79
OpStore %_0_ok %84
%87 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%88 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%89 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%86 = OpCompositeConstruct %mat3v3float %87 %88 %89
%90 = OpVectorTimesMatrix %v3float %39 %86
OpStore %_2_v2 %90
%91 = OpLoad %bool %_0_ok
OpSelectionMerge %93 None
OpBranchConditional %91 %92 %93
%92 = OpLabel
%94 = OpLoad %v3float %_2_v2
%95 = OpFOrdEqual %v3bool %94 %63
%96 = OpAll %bool %95
OpBranch %93
%93 = OpLabel
%97 = OpPhi %bool %false %80 %96 %92
OpStore %_0_ok %97
%98 = OpLoad %bool %_0_ok
OpSelectionMerge %100 None
OpBranchConditional %98 %99 %100
%99 = OpLabel
%101 = OpFunctionCall %bool %test_half_b
OpBranch %100
%100 = OpLabel
%102 = OpPhi %bool %false %93 %101 %99
OpSelectionMerge %107 None
OpBranchConditional %102 %105 %106
%105 = OpLabel
%108 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%112 = OpLoad %v4float %108
OpStore %103 %112
OpBranch %107
%106 = OpLabel
%113 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%115 = OpLoad %v4float %113
OpStore %103 %115
OpBranch %107
%107 = OpLabel
%116 = OpLoad %v4float %103
OpReturnValue %116
OpFunctionEnd
