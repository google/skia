OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInputs"
OpMemberName %_UniformBuffer 1 "colorBlack"
OpMemberName %_UniformBuffer 2 "colorGreen"
OpMemberName %_UniformBuffer 3 "colorRed"
OpName %_entrypoint "_entrypoint"
OpName %main "main"
OpName %_0_non_constant_swizzle "_0_non_constant_swizzle"
OpName %_1_v "_1_v"
OpName %_2_i "_2_i"
OpName %_3_x "_3_x"
OpName %_4_y "_4_y"
OpName %_5_z "_5_z"
OpName %_6_w "_6_w"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
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
OpDecorate %27 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
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
%18 = OpTypeFunction %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%int_1 = OpConstant %int 1
%_ptr_Function_float = OpTypePointer Function %float
%float_n1_25 = OpConstant %float -1.25
%float_0 = OpConstant %float 0
%71 = OpConstantComposite %v4float %float_n1_25 %float_n1_25 %float_n1_25 %float_0
%v4bool = OpTypeVector %bool 4
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%_0_non_constant_swizzle = OpVariable %_ptr_Function_v4float Function
%_1_v = OpVariable %_ptr_Function_v4float Function
%_2_i = OpVariable %_ptr_Function_v4int Function
%_3_x = OpVariable %_ptr_Function_float Function
%_4_y = OpVariable %_ptr_Function_float Function
%_5_z = OpVariable %_ptr_Function_float Function
%_6_w = OpVariable %_ptr_Function_float Function
%75 = OpVariable %_ptr_Function_v4float Function
%23 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%27 = OpLoad %v4float %23
OpStore %_1_v %27
%31 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%33 = OpLoad %v4float %31
%34 = OpCompositeExtract %float %33 0
%35 = OpConvertFToS %int %34
%36 = OpCompositeExtract %float %33 1
%37 = OpConvertFToS %int %36
%38 = OpCompositeExtract %float %33 2
%39 = OpConvertFToS %int %38
%40 = OpCompositeExtract %float %33 3
%41 = OpConvertFToS %int %40
%42 = OpCompositeConstruct %v4int %35 %37 %39 %41
OpStore %_2_i %42
%45 = OpLoad %v4float %_1_v
%46 = OpLoad %v4int %_2_i
%47 = OpCompositeExtract %int %46 0
%48 = OpVectorExtractDynamic %float %45 %47
OpStore %_3_x %48
%50 = OpLoad %v4float %_1_v
%51 = OpLoad %v4int %_2_i
%52 = OpCompositeExtract %int %51 1
%53 = OpVectorExtractDynamic %float %50 %52
OpStore %_4_y %53
%55 = OpLoad %v4float %_1_v
%56 = OpLoad %v4int %_2_i
%57 = OpCompositeExtract %int %56 2
%58 = OpVectorExtractDynamic %float %55 %57
OpStore %_5_z %58
%60 = OpLoad %v4float %_1_v
%61 = OpLoad %v4int %_2_i
%62 = OpCompositeExtract %int %61 3
%63 = OpVectorExtractDynamic %float %60 %62
OpStore %_6_w %63
%64 = OpLoad %float %_3_x
%65 = OpLoad %float %_4_y
%66 = OpLoad %float %_5_z
%67 = OpLoad %float %_6_w
%68 = OpCompositeConstruct %v4float %64 %65 %66 %67
%72 = OpFOrdEqual %v4bool %68 %71
%74 = OpAll %bool %72
OpSelectionMerge %78 None
OpBranchConditional %74 %76 %77
%76 = OpLabel
%79 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%81 = OpLoad %v4float %79
OpStore %75 %81
OpBranch %78
%77 = OpLabel
%82 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%84 = OpLoad %v4float %82
OpStore %75 %84
OpBranch %78
%78 = OpLabel
%85 = OpLoad %v4float %75
OpReturnValue %85
OpFunctionEnd
