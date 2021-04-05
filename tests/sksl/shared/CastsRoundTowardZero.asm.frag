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
OpName %ok "ok"
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
OpDecorate %24 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
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
%18 = OpTypeFunction %v4float
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%v4int = OpTypeVector %int 4
%float_0_00999999978 = OpConstant %float 0.00999999978
%float_0_99000001 = OpConstant %float 0.99000001
%float_1_49000001 = OpConstant %float 1.49000001
%float_2_75 = OpConstant %float 2.75
%v4bool = OpTypeVector %bool 4
%int_n1 = OpConstant %int -1
%int_n2 = OpConstant %int -2
%float_n0_00999999978 = OpConstant %float -0.00999999978
%float_n0_99000001 = OpConstant %float -0.99000001
%float_n1_49000001 = OpConstant %float -1.49000001
%float_n2_75 = OpConstant %float -2.75
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
%111 = OpVariable %_ptr_Function_v4float Function
OpStore %ok %true
%24 = OpLoad %bool %ok
OpSelectionMerge %26 None
OpBranchConditional %24 %25 %26
%25 = OpLabel
%31 = OpCompositeConstruct %v4int %int_0 %int_0 %int_1 %int_2
%33 = OpCompositeExtract %int %31 0
%34 = OpConvertSToF %float %33
%35 = OpCompositeExtract %int %31 1
%36 = OpConvertSToF %float %35
%37 = OpCompositeExtract %int %31 2
%38 = OpConvertSToF %float %37
%39 = OpCompositeExtract %int %31 3
%40 = OpConvertSToF %float %39
%41 = OpCompositeConstruct %v4float %34 %36 %38 %40
%46 = OpCompositeConstruct %v4float %float_0_00999999978 %float_0_99000001 %float_1_49000001 %float_2_75
%47 = OpCompositeExtract %float %46 0
%48 = OpConvertFToS %int %47
%49 = OpCompositeExtract %float %46 1
%50 = OpConvertFToS %int %49
%51 = OpCompositeExtract %float %46 2
%52 = OpConvertFToS %int %51
%53 = OpCompositeExtract %float %46 3
%54 = OpConvertFToS %int %53
%55 = OpCompositeConstruct %v4int %48 %50 %52 %54
%56 = OpCompositeExtract %int %55 0
%57 = OpConvertSToF %float %56
%58 = OpCompositeExtract %int %55 1
%59 = OpConvertSToF %float %58
%60 = OpCompositeExtract %int %55 2
%61 = OpConvertSToF %float %60
%62 = OpCompositeExtract %int %55 3
%63 = OpConvertSToF %float %62
%64 = OpCompositeConstruct %v4float %57 %59 %61 %63
%65 = OpFOrdEqual %v4bool %41 %64
%67 = OpAll %bool %65
OpBranch %26
%26 = OpLabel
%68 = OpPhi %bool %false %19 %67 %25
OpStore %ok %68
%69 = OpLoad %bool %ok
OpSelectionMerge %71 None
OpBranchConditional %69 %70 %71
%70 = OpLabel
%74 = OpCompositeConstruct %v4int %int_0 %int_0 %int_n1 %int_n2
%75 = OpCompositeExtract %int %74 0
%76 = OpConvertSToF %float %75
%77 = OpCompositeExtract %int %74 1
%78 = OpConvertSToF %float %77
%79 = OpCompositeExtract %int %74 2
%80 = OpConvertSToF %float %79
%81 = OpCompositeExtract %int %74 3
%82 = OpConvertSToF %float %81
%83 = OpCompositeConstruct %v4float %76 %78 %80 %82
%88 = OpCompositeConstruct %v4float %float_n0_00999999978 %float_n0_99000001 %float_n1_49000001 %float_n2_75
%89 = OpCompositeExtract %float %88 0
%90 = OpConvertFToS %int %89
%91 = OpCompositeExtract %float %88 1
%92 = OpConvertFToS %int %91
%93 = OpCompositeExtract %float %88 2
%94 = OpConvertFToS %int %93
%95 = OpCompositeExtract %float %88 3
%96 = OpConvertFToS %int %95
%97 = OpCompositeConstruct %v4int %90 %92 %94 %96
%98 = OpCompositeExtract %int %97 0
%99 = OpConvertSToF %float %98
%100 = OpCompositeExtract %int %97 1
%101 = OpConvertSToF %float %100
%102 = OpCompositeExtract %int %97 2
%103 = OpConvertSToF %float %102
%104 = OpCompositeExtract %int %97 3
%105 = OpConvertSToF %float %104
%106 = OpCompositeConstruct %v4float %99 %101 %103 %105
%107 = OpFOrdEqual %v4bool %83 %106
%108 = OpAll %bool %107
OpBranch %71
%71 = OpLabel
%109 = OpPhi %bool %false %26 %108 %70
OpStore %ok %109
%110 = OpLoad %bool %ok
OpSelectionMerge %115 None
OpBranchConditional %110 %113 %114
%113 = OpLabel
%116 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%118 = OpLoad %v4float %116
OpStore %111 %118
OpBranch %115
%114 = OpLabel
%119 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%120 = OpLoad %v4float %119
OpStore %111 %120
OpBranch %115
%115 = OpLabel
%121 = OpLoad %v4float %111
OpReturnValue %121
OpFunctionEnd
