OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint "_entrypoint"
OpName %main "main"
OpName %x "x"
OpName %y "y"
OpName %z "z"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %62 RelaxedPrecision
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
%_ptr_Function_float = OpTypePointer Function %float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_3 = OpConstant %int 3
%float_0_5 = OpConstant %float 0.5
%int_8 = OpConstant %int 8
%float_12 = OpConstant %float 12
%float_10 = OpConstant %float 10
%int_0 = OpConstant %int 0
%int_n1 = OpConstant %int -1
%int_2 = OpConstant %int 2
%int_4 = OpConstant %int 4
%int_5 = OpConstant %int 5
%float_6 = OpConstant %float 6
%int_6 = OpConstant %int 6
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%x = OpVariable %_ptr_Function_float Function
%y = OpVariable %_ptr_Function_float Function
%z = OpVariable %_ptr_Function_int Function
OpStore %x %float_1
OpStore %y %float_2
OpStore %z %int_3
OpStore %x %float_2
OpStore %y %float_0_5
OpStore %z %int_8
%31 = OpLoad %float %x
%33 = OpFAdd %float %31 %float_12
OpStore %x %33
%34 = OpLoad %float %x
%35 = OpFSub %float %34 %float_12
OpStore %x %35
%36 = OpLoad %float %x
%37 = OpLoad %float %y
%39 = OpFDiv %float %37 %float_10
OpStore %y %39
%40 = OpFMul %float %36 %39
OpStore %x %40
%41 = OpLoad %int %z
%43 = OpBitwiseOr %int %41 %int_0
OpStore %z %43
%44 = OpLoad %int %z
%46 = OpBitwiseAnd %int %44 %int_n1
OpStore %z %46
%47 = OpLoad %int %z
%48 = OpBitwiseXor %int %47 %int_0
OpStore %z %48
%49 = OpLoad %int %z
%51 = OpShiftRightArithmetic %int %49 %int_2
OpStore %z %51
%52 = OpLoad %int %z
%54 = OpShiftLeftLogical %int %52 %int_4
OpStore %z %54
%55 = OpLoad %int %z
%57 = OpSMod %int %55 %int_5
OpStore %z %57
OpStore %x %float_6
OpStore %y %float_6
OpStore %z %int_6
%60 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%62 = OpLoad %v4float %60
OpReturnValue %62
OpFunctionEnd
