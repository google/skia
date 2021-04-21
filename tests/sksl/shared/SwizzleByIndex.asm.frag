OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInputs"
OpMemberName %_UniformBuffer 1 "colorBlack"
OpMemberName %_UniformBuffer 2 "colorGreen"
OpMemberName %_UniformBuffer 3 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %_0_v "_0_v"
OpName %_1_i "_1_i"
OpName %_2_x "_2_x"
OpName %_3_y "_3_y"
OpName %_4_z "_4_z"
OpName %_5_w "_5_w"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 3 Offset 48
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %_0_v RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %_2_x RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %_3_y RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %_4_z RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %_5_w RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%int_1 = OpConstant %int 1
%_ptr_Function_float = OpTypePointer Function %float
%float_n1_25 = OpConstant %float -1.25
%75 = OpConstantComposite %v4float %float_n1_25 %float_n1_25 %float_n1_25 %float_0
%v4bool = OpTypeVector %bool 4
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
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
%_0_v = OpVariable %_ptr_Function_v4float Function
%_1_i = OpVariable %_ptr_Function_v4int Function
%_2_x = OpVariable %_ptr_Function_float Function
%_3_y = OpVariable %_ptr_Function_float Function
%_4_z = OpVariable %_ptr_Function_float Function
%_5_w = OpVariable %_ptr_Function_float Function
%79 = OpVariable %_ptr_Function_v4float Function
%28 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%32 = OpLoad %v4float %28
OpStore %_0_v %32
%36 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%38 = OpLoad %v4float %36
%39 = OpCompositeExtract %float %38 0
%40 = OpConvertFToS %int %39
%41 = OpCompositeExtract %float %38 1
%42 = OpConvertFToS %int %41
%43 = OpCompositeExtract %float %38 2
%44 = OpConvertFToS %int %43
%45 = OpCompositeExtract %float %38 3
%46 = OpConvertFToS %int %45
%47 = OpCompositeConstruct %v4int %40 %42 %44 %46
OpStore %_1_i %47
%50 = OpLoad %v4float %_0_v
%51 = OpLoad %v4int %_1_i
%52 = OpCompositeExtract %int %51 0
%53 = OpVectorExtractDynamic %float %50 %52
OpStore %_2_x %53
%55 = OpLoad %v4float %_0_v
%56 = OpLoad %v4int %_1_i
%57 = OpCompositeExtract %int %56 1
%58 = OpVectorExtractDynamic %float %55 %57
OpStore %_3_y %58
%60 = OpLoad %v4float %_0_v
%61 = OpLoad %v4int %_1_i
%62 = OpCompositeExtract %int %61 2
%63 = OpVectorExtractDynamic %float %60 %62
OpStore %_4_z %63
%65 = OpLoad %v4float %_0_v
%66 = OpLoad %v4int %_1_i
%67 = OpCompositeExtract %int %66 3
%68 = OpVectorExtractDynamic %float %65 %67
OpStore %_5_w %68
%69 = OpLoad %float %_2_x
%70 = OpLoad %float %_3_y
%71 = OpLoad %float %_4_z
%72 = OpLoad %float %_5_w
%73 = OpCompositeConstruct %v4float %69 %70 %71 %72
%76 = OpFOrdEqual %v4bool %73 %75
%78 = OpAll %bool %76
OpSelectionMerge %82 None
OpBranchConditional %78 %80 %81
%80 = OpLabel
%83 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%85 = OpLoad %v4float %83
OpStore %79 %85
OpBranch %82
%81 = OpLabel
%86 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%88 = OpLoad %v4float %86
OpStore %79 %88
OpBranch %82
%82 = OpLabel
%89 = OpLoad %v4float %79
OpReturnValue %89
OpFunctionEnd
