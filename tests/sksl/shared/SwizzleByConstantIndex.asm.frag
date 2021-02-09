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
OpName %constant_swizzle "constant_swizzle"
OpName %v "v"
OpName %x "x"
OpName %y "y"
OpName %z "z"
OpName %w "w"
OpName %foldable "foldable"
OpName %v_0 "v"
OpName %x_0 "x"
OpName %y_0 "y"
OpName %z_0 "z"
OpName %w_0 "w"
OpName %main "main"
OpName %a "a"
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
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %28 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%20 = OpTypeFunction %v4float
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
%81 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0_75 %float_2_25
%v4bool = OpTypeVector %bool 4
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%_entrypoint = OpFunction %void None %17
%18 = OpLabel
%19 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %19
OpReturn
OpFunctionEnd
%constant_swizzle = OpFunction %v4float None %20
%21 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%x = OpVariable %_ptr_Function_float Function
%y = OpVariable %_ptr_Function_float Function
%z = OpVariable %_ptr_Function_float Function
%w = OpVariable %_ptr_Function_float Function
%24 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%28 = OpLoad %v4float %24
OpStore %v %28
%31 = OpLoad %v4float %v
%32 = OpCompositeExtract %float %31 0
OpStore %x %32
%34 = OpLoad %v4float %v
%35 = OpCompositeExtract %float %34 1
OpStore %y %35
%37 = OpLoad %v4float %v
%38 = OpCompositeExtract %float %37 2
OpStore %z %38
%40 = OpLoad %v4float %v
%41 = OpCompositeExtract %float %40 3
OpStore %w %41
%42 = OpLoad %float %x
%43 = OpLoad %float %y
%44 = OpLoad %float %z
%45 = OpLoad %float %w
%46 = OpCompositeConstruct %v4float %42 %43 %44 %45
OpReturnValue %46
OpFunctionEnd
%foldable = OpFunction %v4float None %20
%47 = OpLabel
%v_0 = OpVariable %_ptr_Function_v4float Function
%x_0 = OpVariable %_ptr_Function_float Function
%y_0 = OpVariable %_ptr_Function_float Function
%z_0 = OpVariable %_ptr_Function_float Function
%w_0 = OpVariable %_ptr_Function_float Function
OpStore %v_0 %53
%55 = OpLoad %v4float %v_0
%56 = OpCompositeExtract %float %55 0
OpStore %x_0 %56
%58 = OpLoad %v4float %v_0
%59 = OpCompositeExtract %float %58 1
OpStore %y_0 %59
%61 = OpLoad %v4float %v_0
%62 = OpCompositeExtract %float %61 2
OpStore %z_0 %62
%64 = OpLoad %v4float %v_0
%65 = OpCompositeExtract %float %64 3
OpStore %w_0 %65
%66 = OpLoad %float %x_0
%67 = OpLoad %float %y_0
%68 = OpLoad %float %z_0
%69 = OpLoad %float %w_0
%70 = OpCompositeConstruct %v4float %66 %67 %68 %69
OpReturnValue %70
OpFunctionEnd
%main = OpFunction %v4float None %20
%71 = OpLabel
%a = OpVariable %_ptr_Function_v4float Function
%b = OpVariable %_ptr_Function_v4float Function
%91 = OpVariable %_ptr_Function_v4float Function
%73 = OpFunctionCall %v4float %constant_swizzle
OpStore %a %73
%75 = OpFunctionCall %v4float %foldable
OpStore %b %75
%77 = OpLoad %v4float %a
%82 = OpFOrdEqual %v4bool %77 %81
%84 = OpAll %bool %82
OpSelectionMerge %86 None
OpBranchConditional %84 %85 %86
%85 = OpLabel
%87 = OpLoad %v4float %b
%88 = OpFOrdEqual %v4bool %87 %53
%89 = OpAll %bool %88
OpBranch %86
%86 = OpLabel
%90 = OpPhi %bool %false %71 %89 %85
OpSelectionMerge %94 None
OpBranchConditional %90 %92 %93
%92 = OpLabel
%95 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%97 = OpLoad %v4float %95
OpStore %91 %97
OpBranch %94
%93 = OpLabel
%98 = OpAccessChain %_ptr_Uniform_v4float %12 %int_2
%100 = OpLoad %v4float %98
OpStore %91 %100
OpBranch %94
%94 = OpLabel
%101 = OpLoad %v4float %91
OpReturnValue %101
OpFunctionEnd
