OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInputs"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpName %_entrypoint "_entrypoint"
OpName %main "main"
OpName %_0_constant_swizzle "_0_constant_swizzle"
OpName %_1_v "_1_v"
OpName %_2_x "_2_x"
OpName %_3_y "_3_y"
OpName %_4_z "_4_z"
OpName %_5_w "_5_w"
OpName %a "a"
OpName %_6_foldable "_6_foldable"
OpName %_7_v "_7_v"
OpName %_8_x "_8_x"
OpName %_9_y "_9_y"
OpName %_10_z "_10_z"
OpName %_11_w "_11_w"
OpName %b "b"
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
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %27 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%18 = OpTypeFunction %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
%float_0 = OpConstant %float 0
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%53 = OpConstantComposite %v4float %float_0 %float_1 %float_2 %float_3
%false = OpConstantFalse %bool
%float_n1_25 = OpConstant %float -1.25
%float_0_75 = OpConstant %float 0.75
%float_2_25 = OpConstant %float 2.25
%77 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0_75 %float_2_25
%v4bool = OpTypeVector %bool 4
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%_0_constant_swizzle = OpVariable %_ptr_Function_v4float Function
%_1_v = OpVariable %_ptr_Function_v4float Function
%_2_x = OpVariable %_ptr_Function_float Function
%_3_y = OpVariable %_ptr_Function_float Function
%_4_z = OpVariable %_ptr_Function_float Function
%_5_w = OpVariable %_ptr_Function_float Function
%a = OpVariable %_ptr_Function_v4float Function
%_6_foldable = OpVariable %_ptr_Function_v4float Function
%_7_v = OpVariable %_ptr_Function_v4float Function
%_8_x = OpVariable %_ptr_Function_float Function
%_9_y = OpVariable %_ptr_Function_float Function
%_10_z = OpVariable %_ptr_Function_float Function
%_11_w = OpVariable %_ptr_Function_float Function
%b = OpVariable %_ptr_Function_v4float Function
%87 = OpVariable %_ptr_Function_v4float Function
%23 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%27 = OpLoad %v4float %23
OpStore %_1_v %27
%30 = OpLoad %v4float %_1_v
%31 = OpCompositeExtract %float %30 0
OpStore %_2_x %31
%33 = OpLoad %v4float %_1_v
%34 = OpCompositeExtract %float %33 1
OpStore %_3_y %34
%36 = OpLoad %v4float %_1_v
%37 = OpCompositeExtract %float %36 2
OpStore %_4_z %37
%39 = OpLoad %v4float %_1_v
%40 = OpCompositeExtract %float %39 3
OpStore %_5_w %40
%42 = OpLoad %float %_2_x
%43 = OpLoad %float %_3_y
%44 = OpLoad %float %_4_z
%45 = OpLoad %float %_5_w
%46 = OpCompositeConstruct %v4float %42 %43 %44 %45
OpStore %a %46
OpStore %_7_v %53
%55 = OpLoad %v4float %_7_v
%56 = OpCompositeExtract %float %55 0
OpStore %_8_x %56
%58 = OpLoad %v4float %_7_v
%59 = OpCompositeExtract %float %58 1
OpStore %_9_y %59
%61 = OpLoad %v4float %_7_v
%62 = OpCompositeExtract %float %61 2
OpStore %_10_z %62
%64 = OpLoad %v4float %_7_v
%65 = OpCompositeExtract %float %64 3
OpStore %_11_w %65
%67 = OpLoad %float %_8_x
%68 = OpLoad %float %_9_y
%69 = OpLoad %float %_10_z
%70 = OpLoad %float %_11_w
%71 = OpCompositeConstruct %v4float %67 %68 %69 %70
OpStore %b %71
%73 = OpLoad %v4float %a
%78 = OpFOrdEqual %v4bool %73 %77
%80 = OpAll %bool %78
OpSelectionMerge %82 None
OpBranchConditional %80 %81 %82
%81 = OpLabel
%83 = OpLoad %v4float %b
%84 = OpFOrdEqual %v4bool %83 %53
%85 = OpAll %bool %84
OpBranch %82
%82 = OpLabel
%86 = OpPhi %bool %false %19 %85 %81
OpSelectionMerge %90 None
OpBranchConditional %86 %88 %89
%88 = OpLabel
%91 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%93 = OpLoad %v4float %91
OpStore %87 %93
OpBranch %90
%89 = OpLabel
%94 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%96 = OpLoad %v4float %94
OpStore %87 %96
OpBranch %90
%90 = OpLabel
%97 = OpLoad %v4float %87
OpReturnValue %97
OpFunctionEnd
