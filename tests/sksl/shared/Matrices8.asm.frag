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
OpName %m10 "m10"
OpName %main "main"
OpName %_0_ok "_0_ok"
OpName %_1_m10 "_1_m10"
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
OpDecorate %m10 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
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
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%float_11 = OpConstant %float 11
%false = OpConstantFalse %bool
%v4bool = OpTypeVector %bool 4
%70 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%m10 = OpVariable %_ptr_Function_mat4v4float Function
OpStore %ok %true
%34 = OpCompositeConstruct %v4float %float_11 %float_0 %float_0 %float_0
%35 = OpCompositeConstruct %v4float %float_0 %float_11 %float_0 %float_0
%36 = OpCompositeConstruct %v4float %float_0 %float_0 %float_11 %float_0
%37 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_11
%33 = OpCompositeConstruct %mat4v4float %34 %35 %36 %37
OpStore %m10 %33
%39 = OpLoad %bool %ok
OpSelectionMerge %41 None
OpBranchConditional %39 %40 %41
%40 = OpLabel
%42 = OpLoad %mat4v4float %m10
%44 = OpCompositeConstruct %v4float %float_11 %float_0 %float_0 %float_0
%45 = OpCompositeConstruct %v4float %float_0 %float_11 %float_0 %float_0
%46 = OpCompositeConstruct %v4float %float_0 %float_0 %float_11 %float_0
%47 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_11
%43 = OpCompositeConstruct %mat4v4float %44 %45 %46 %47
%49 = OpCompositeExtract %v4float %42 0
%50 = OpCompositeExtract %v4float %43 0
%51 = OpFOrdEqual %v4bool %49 %50
%52 = OpAll %bool %51
%53 = OpCompositeExtract %v4float %42 1
%54 = OpCompositeExtract %v4float %43 1
%55 = OpFOrdEqual %v4bool %53 %54
%56 = OpAll %bool %55
%57 = OpLogicalAnd %bool %52 %56
%58 = OpCompositeExtract %v4float %42 2
%59 = OpCompositeExtract %v4float %43 2
%60 = OpFOrdEqual %v4bool %58 %59
%61 = OpAll %bool %60
%62 = OpLogicalAnd %bool %57 %61
%63 = OpCompositeExtract %v4float %42 3
%64 = OpCompositeExtract %v4float %43 3
%65 = OpFOrdEqual %v4bool %63 %64
%66 = OpAll %bool %65
%67 = OpLogicalAnd %bool %62 %66
OpBranch %41
%41 = OpLabel
%68 = OpPhi %bool %false %25 %67 %40
OpStore %ok %68
%69 = OpLoad %bool %ok
OpReturnValue %69
OpFunctionEnd
%main = OpFunction %v4float None %70
%71 = OpFunctionParameter %_ptr_Function_v2float
%72 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m10 = OpVariable %_ptr_Function_mat4v4float Function
%114 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%76 = OpCompositeConstruct %v4float %float_11 %float_0 %float_0 %float_0
%77 = OpCompositeConstruct %v4float %float_0 %float_11 %float_0 %float_0
%78 = OpCompositeConstruct %v4float %float_0 %float_0 %float_11 %float_0
%79 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_11
%75 = OpCompositeConstruct %mat4v4float %76 %77 %78 %79
OpStore %_1_m10 %75
%80 = OpLoad %bool %_0_ok
OpSelectionMerge %82 None
OpBranchConditional %80 %81 %82
%81 = OpLabel
%83 = OpLoad %mat4v4float %_1_m10
%85 = OpCompositeConstruct %v4float %float_11 %float_0 %float_0 %float_0
%86 = OpCompositeConstruct %v4float %float_0 %float_11 %float_0 %float_0
%87 = OpCompositeConstruct %v4float %float_0 %float_0 %float_11 %float_0
%88 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_11
%84 = OpCompositeConstruct %mat4v4float %85 %86 %87 %88
%89 = OpCompositeExtract %v4float %83 0
%90 = OpCompositeExtract %v4float %84 0
%91 = OpFOrdEqual %v4bool %89 %90
%92 = OpAll %bool %91
%93 = OpCompositeExtract %v4float %83 1
%94 = OpCompositeExtract %v4float %84 1
%95 = OpFOrdEqual %v4bool %93 %94
%96 = OpAll %bool %95
%97 = OpLogicalAnd %bool %92 %96
%98 = OpCompositeExtract %v4float %83 2
%99 = OpCompositeExtract %v4float %84 2
%100 = OpFOrdEqual %v4bool %98 %99
%101 = OpAll %bool %100
%102 = OpLogicalAnd %bool %97 %101
%103 = OpCompositeExtract %v4float %83 3
%104 = OpCompositeExtract %v4float %84 3
%105 = OpFOrdEqual %v4bool %103 %104
%106 = OpAll %bool %105
%107 = OpLogicalAnd %bool %102 %106
OpBranch %82
%82 = OpLabel
%108 = OpPhi %bool %false %72 %107 %81
OpStore %_0_ok %108
%109 = OpLoad %bool %_0_ok
OpSelectionMerge %111 None
OpBranchConditional %109 %110 %111
%110 = OpLabel
%112 = OpFunctionCall %bool %test_half_b
OpBranch %111
%111 = OpLabel
%113 = OpPhi %bool %false %82 %112 %110
OpSelectionMerge %118 None
OpBranchConditional %113 %116 %117
%116 = OpLabel
%119 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%123 = OpLoad %v4float %119
OpStore %114 %123
OpBranch %118
%117 = OpLabel
%124 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%126 = OpLoad %v4float %124
OpStore %114 %126
OpBranch %118
%118 = OpLabel
%127 = OpLoad %v4float %114
OpReturnValue %127
OpFunctionEnd
