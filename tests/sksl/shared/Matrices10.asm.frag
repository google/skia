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
OpName %v2 "v2"
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
OpDecorate %v2 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
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
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%float_3 = OpConstant %float 3
%30 = OpConstantComposite %v3float %float_3 %float_3 %float_3
%mat3v3float = OpTypeMatrix %v3float 3
%false = OpConstantFalse %bool
%float_8_99999046 = OpConstant %float 8.99999046
%42 = OpConstantComposite %v3float %float_8_99999046 %float_8_99999046 %float_8_99999046
%v3bool = OpTypeVector %bool 3
%float_9_00000954 = OpConstant %float 9.00000954
%50 = OpConstantComposite %v3float %float_9_00000954 %float_9_00000954 %float_9_00000954
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%int_0 = OpConstant %int 0
%float_8_99989986 = OpConstant %float 8.99989986
%61 = OpConstantComposite %v3float %float_8_99989986 %float_8_99989986 %float_8_99989986
%float_9_00010014 = OpConstant %float 9.00010014
%68 = OpConstantComposite %v3float %float_9_00010014 %float_9_00010014 %float_9_00010014
%float_8_9989996 = OpConstant %float 8.9989996
%76 = OpConstantComposite %v3float %float_8_9989996 %float_8_9989996 %float_8_9989996
%float_9_0010004 = OpConstant %float 9.0010004
%83 = OpConstantComposite %v3float %float_9_0010004 %float_9_0010004 %float_9_0010004
%float_8_98999977 = OpConstant %float 8.98999977
%91 = OpConstantComposite %v3float %float_8_98999977 %float_8_98999977 %float_8_98999977
%float_9_01000023 = OpConstant %float 9.01000023
%98 = OpConstantComposite %v3float %float_9_01000023 %float_9_01000023 %float_9_01000023
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
%v2 = OpVariable %_ptr_Function_v3float Function
%32 = OpCompositeConstruct %v3float %float_3 %float_0 %float_0
%33 = OpCompositeConstruct %v3float %float_0 %float_3 %float_0
%34 = OpCompositeConstruct %v3float %float_0 %float_0 %float_3
%31 = OpCompositeConstruct %mat3v3float %32 %33 %34
%36 = OpVectorTimesMatrix %v3float %30 %31
OpStore %v2 %36
%40 = OpLoad %v3float %v2
%39 = OpFOrdGreaterThanEqual %v3bool %40 %42
%38 = OpAll %bool %39
OpSelectionMerge %45 None
OpBranchConditional %38 %44 %45
%44 = OpLabel
%48 = OpLoad %v3float %v2
%47 = OpFOrdLessThanEqual %v3bool %48 %50
%46 = OpAll %bool %47
OpBranch %45
%45 = OpLabel
%51 = OpPhi %bool %false %25 %46 %44
%52 = OpSelect %int %51 %int_1 %int_0
%56 = OpConvertSToF %float %52
%59 = OpLoad %v3float %v2
%58 = OpFOrdGreaterThanEqual %v3bool %59 %61
%57 = OpAll %bool %58
OpSelectionMerge %63 None
OpBranchConditional %57 %62 %63
%62 = OpLabel
%66 = OpLoad %v3float %v2
%65 = OpFOrdLessThanEqual %v3bool %66 %68
%64 = OpAll %bool %65
OpBranch %63
%63 = OpLabel
%69 = OpPhi %bool %false %45 %64 %62
%70 = OpSelect %int %69 %int_1 %int_0
%71 = OpConvertSToF %float %70
%74 = OpLoad %v3float %v2
%73 = OpFOrdGreaterThanEqual %v3bool %74 %76
%72 = OpAll %bool %73
OpSelectionMerge %78 None
OpBranchConditional %72 %77 %78
%77 = OpLabel
%81 = OpLoad %v3float %v2
%80 = OpFOrdLessThanEqual %v3bool %81 %83
%79 = OpAll %bool %80
OpBranch %78
%78 = OpLabel
%84 = OpPhi %bool %false %63 %79 %77
%85 = OpSelect %int %84 %int_1 %int_0
%86 = OpConvertSToF %float %85
%89 = OpLoad %v3float %v2
%88 = OpFOrdGreaterThanEqual %v3bool %89 %91
%87 = OpAll %bool %88
OpSelectionMerge %93 None
OpBranchConditional %87 %92 %93
%92 = OpLabel
%96 = OpLoad %v3float %v2
%95 = OpFOrdLessThanEqual %v3bool %96 %98
%94 = OpAll %bool %95
OpBranch %93
%93 = OpLabel
%99 = OpPhi %bool %false %78 %94 %92
%100 = OpSelect %int %99 %int_1 %int_0
%101 = OpConvertSToF %float %100
%102 = OpCompositeConstruct %v4float %56 %71 %86 %101
OpReturnValue %102
OpFunctionEnd
