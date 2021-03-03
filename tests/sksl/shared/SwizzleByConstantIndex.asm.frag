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
OpName %_0_v "_0_v"
OpName %_1_x "_1_x"
OpName %_2_y "_2_y"
OpName %_3_z "_3_z"
OpName %_4_w "_4_w"
OpName %a "a"
OpName %_5_v "_5_v"
OpName %_6_x "_6_x"
OpName %_7_y "_7_y"
OpName %_8_z "_8_z"
OpName %_9_w "_9_w"
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
OpDecorate %26 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
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
%51 = OpConstantComposite %v4float %float_0 %float_1 %float_2 %float_3
%false = OpConstantFalse %bool
%float_n1_25 = OpConstant %float -1.25
%float_0_75 = OpConstant %float 0.75
%float_2_25 = OpConstant %float 2.25
%75 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0_75 %float_2_25
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
%_0_v = OpVariable %_ptr_Function_v4float Function
%_1_x = OpVariable %_ptr_Function_float Function
%_2_y = OpVariable %_ptr_Function_float Function
%_3_z = OpVariable %_ptr_Function_float Function
%_4_w = OpVariable %_ptr_Function_float Function
%a = OpVariable %_ptr_Function_v4float Function
%_5_v = OpVariable %_ptr_Function_v4float Function
%_6_x = OpVariable %_ptr_Function_float Function
%_7_y = OpVariable %_ptr_Function_float Function
%_8_z = OpVariable %_ptr_Function_float Function
%_9_w = OpVariable %_ptr_Function_float Function
%b = OpVariable %_ptr_Function_v4float Function
%85 = OpVariable %_ptr_Function_v4float Function
%22 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%26 = OpLoad %v4float %22
OpStore %_0_v %26
%29 = OpLoad %v4float %_0_v
%30 = OpCompositeExtract %float %29 0
OpStore %_1_x %30
%32 = OpLoad %v4float %_0_v
%33 = OpCompositeExtract %float %32 1
OpStore %_2_y %33
%35 = OpLoad %v4float %_0_v
%36 = OpCompositeExtract %float %35 2
OpStore %_3_z %36
%38 = OpLoad %v4float %_0_v
%39 = OpCompositeExtract %float %38 3
OpStore %_4_w %39
%41 = OpLoad %float %_1_x
%42 = OpLoad %float %_2_y
%43 = OpLoad %float %_3_z
%44 = OpLoad %float %_4_w
%45 = OpCompositeConstruct %v4float %41 %42 %43 %44
OpStore %a %45
OpStore %_5_v %51
%53 = OpLoad %v4float %_5_v
%54 = OpCompositeExtract %float %53 0
OpStore %_6_x %54
%56 = OpLoad %v4float %_5_v
%57 = OpCompositeExtract %float %56 1
OpStore %_7_y %57
%59 = OpLoad %v4float %_5_v
%60 = OpCompositeExtract %float %59 2
OpStore %_8_z %60
%62 = OpLoad %v4float %_5_v
%63 = OpCompositeExtract %float %62 3
OpStore %_9_w %63
%65 = OpLoad %float %_6_x
%66 = OpLoad %float %_7_y
%67 = OpLoad %float %_8_z
%68 = OpLoad %float %_9_w
%69 = OpCompositeConstruct %v4float %65 %66 %67 %68
OpStore %b %69
%71 = OpLoad %v4float %a
%76 = OpFOrdEqual %v4bool %71 %75
%78 = OpAll %bool %76
OpSelectionMerge %80 None
OpBranchConditional %78 %79 %80
%79 = OpLabel
%81 = OpLoad %v4float %b
%82 = OpFOrdEqual %v4bool %81 %51
%83 = OpAll %bool %82
OpBranch %80
%80 = OpLabel
%84 = OpPhi %bool %false %19 %83 %79
OpSelectionMerge %88 None
OpBranchConditional %84 %86 %87
%86 = OpLabel
%89 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%91 = OpLoad %v4float %89
OpStore %85 %91
OpBranch %88
%87 = OpLabel
%92 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%94 = OpLoad %v4float %92
OpStore %85 %94
OpBranch %88
%88 = OpLabel
%95 = OpLoad %v4float %85
OpReturnValue %95
OpFunctionEnd
