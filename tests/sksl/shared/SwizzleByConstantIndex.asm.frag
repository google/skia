OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInputs"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %constant_swizzle_h4 "constant_swizzle_h4"
OpName %v "v"
OpName %x "x"
OpName %y "y"
OpName %z "z"
OpName %w "w"
OpName %foldable_index_h4 "foldable_index_h4"
OpName %x_0 "x"
OpName %y_0 "y"
OpName %z_0 "z"
OpName %w_0 "w"
OpName %foldable_h4 "foldable_h4"
OpName %v_0 "v"
OpName %x_1 "x"
OpName %y_1 "y"
OpName %z_1 "z"
OpName %w_1 "w"
OpName %main "main"
OpName %a "a"
OpName %b "b"
OpName %c "c"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %13 Binding 0
OpDecorate %13 DescriptorSet 0
OpDecorate %v RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %x RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %y RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %z RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %w RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %x_0 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %y_0 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %z_0 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %w_0 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %v_0 RelaxedPrecision
OpDecorate %x_1 RelaxedPrecision
OpDecorate %y_1 RelaxedPrecision
OpDecorate %z_1 RelaxedPrecision
OpDecorate %w_1 RelaxedPrecision
OpDecorate %a RelaxedPrecision
OpDecorate %b RelaxedPrecision
OpDecorate %c RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%13 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%18 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%26 = OpTypeFunction %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_float = OpTypePointer Function %float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%68 = OpConstantComposite %v4float %float_0 %float_1 %float_2 %float_3
%73 = OpTypeFunction %v4float %_ptr_Function_v2float
%false = OpConstantFalse %bool
%float_n1_25 = OpConstant %float -1.25
%float_0_75 = OpConstant %float 0.75
%float_2_25 = OpConstant %float 2.25
%86 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0_75 %float_2_25
%v4bool = OpTypeVector %bool 4
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%_entrypoint_v = OpFunction %void None %18
%19 = OpLabel
%23 = OpVariable %_ptr_Function_v2float Function
OpStore %23 %22
%25 = OpFunctionCall %v4float %main %23
OpStore %sk_FragColor %25
OpReturn
OpFunctionEnd
%constant_swizzle_h4 = OpFunction %v4float None %26
%27 = OpLabel
%v = OpVariable %_ptr_Function_v4float Function
%x = OpVariable %_ptr_Function_float Function
%y = OpVariable %_ptr_Function_float Function
%z = OpVariable %_ptr_Function_float Function
%w = OpVariable %_ptr_Function_float Function
%30 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%34 = OpLoad %v4float %30
OpStore %v %34
%37 = OpCompositeExtract %float %34 0
OpStore %x %37
%39 = OpCompositeExtract %float %34 1
OpStore %y %39
%41 = OpCompositeExtract %float %34 2
OpStore %z %41
%43 = OpCompositeExtract %float %34 3
OpStore %w %43
%44 = OpCompositeConstruct %v4float %37 %39 %41 %43
OpReturnValue %44
OpFunctionEnd
%foldable_index_h4 = OpFunction %v4float None %26
%45 = OpLabel
%x_0 = OpVariable %_ptr_Function_float Function
%y_0 = OpVariable %_ptr_Function_float Function
%z_0 = OpVariable %_ptr_Function_float Function
%w_0 = OpVariable %_ptr_Function_float Function
%47 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%48 = OpLoad %v4float %47
%49 = OpCompositeExtract %float %48 0
OpStore %x_0 %49
%51 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%52 = OpLoad %v4float %51
%53 = OpCompositeExtract %float %52 1
OpStore %y_0 %53
%55 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%56 = OpLoad %v4float %55
%57 = OpCompositeExtract %float %56 2
OpStore %z_0 %57
%59 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%60 = OpLoad %v4float %59
%61 = OpCompositeExtract %float %60 3
OpStore %w_0 %61
%62 = OpCompositeConstruct %v4float %49 %53 %57 %61
OpReturnValue %62
OpFunctionEnd
%foldable_h4 = OpFunction %v4float None %26
%63 = OpLabel
%v_0 = OpVariable %_ptr_Function_v4float Function
%x_1 = OpVariable %_ptr_Function_float Function
%y_1 = OpVariable %_ptr_Function_float Function
%z_1 = OpVariable %_ptr_Function_float Function
%w_1 = OpVariable %_ptr_Function_float Function
OpStore %v_0 %68
OpStore %x_1 %float_0
OpStore %y_1 %float_1
OpStore %z_1 %float_2
OpStore %w_1 %float_3
OpReturnValue %68
OpFunctionEnd
%main = OpFunction %v4float None %73
%74 = OpFunctionParameter %_ptr_Function_v2float
%75 = OpLabel
%a = OpVariable %_ptr_Function_v4float Function
%b = OpVariable %_ptr_Function_v4float Function
%c = OpVariable %_ptr_Function_v4float Function
%100 = OpVariable %_ptr_Function_v4float Function
%77 = OpFunctionCall %v4float %constant_swizzle_h4
OpStore %a %77
%79 = OpFunctionCall %v4float %foldable_index_h4
OpStore %b %79
%81 = OpFunctionCall %v4float %foldable_h4
OpStore %c %81
%87 = OpFOrdEqual %v4bool %77 %86
%89 = OpAll %bool %87
OpSelectionMerge %91 None
OpBranchConditional %89 %90 %91
%90 = OpLabel
%92 = OpFOrdEqual %v4bool %79 %86
%93 = OpAll %bool %92
OpBranch %91
%91 = OpLabel
%94 = OpPhi %bool %false %75 %93 %90
OpSelectionMerge %96 None
OpBranchConditional %94 %95 %96
%95 = OpLabel
%97 = OpFOrdEqual %v4bool %81 %68
%98 = OpAll %bool %97
OpBranch %96
%96 = OpLabel
%99 = OpPhi %bool %false %91 %98 %95
OpSelectionMerge %103 None
OpBranchConditional %99 %101 %102
%101 = OpLabel
%104 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%106 = OpLoad %v4float %104
OpStore %100 %106
OpBranch %103
%102 = OpLabel
%107 = OpAccessChain %_ptr_Uniform_v4float %13 %int_2
%109 = OpLoad %v4float %107
OpStore %100 %109
OpBranch %103
%103 = OpLabel
%110 = OpLoad %v4float %100
OpReturnValue %110
OpFunctionEnd
