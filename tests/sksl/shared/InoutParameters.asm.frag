OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpName %_entrypoint_v "_entrypoint_v"
OpName %outParameterWrite_vh4 "outParameterWrite_vh4"
OpName %outParameterWriteIndirect_vh4 "outParameterWriteIndirect_vh4"
OpName %inoutParameterWrite_vh4 "inoutParameterWrite_vh4"
OpName %inoutParameterWriteIndirect_vh4 "inoutParameterWriteIndirect_vh4"
OpName %main "main"
OpName %c "c"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %14 Binding 0
OpDecorate %14 DescriptorSet 0
OpDecorate %35 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %c RelaxedPrecision
OpDecorate %56 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%14 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%19 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%23 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%27 = OpTypeFunction %void %_ptr_Function_v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%47 = OpTypeFunction %v4float %_ptr_Function_v2float
%false = OpConstantFalse %bool
%_entrypoint_v = OpFunction %void None %19
%20 = OpLabel
%24 = OpVariable %_ptr_Function_v2float Function
OpStore %24 %23
%26 = OpFunctionCall %v4float %main %24
OpStore %sk_FragColor %26
OpReturn
OpFunctionEnd
%outParameterWrite_vh4 = OpFunction %void None %27
%29 = OpFunctionParameter %_ptr_Function_v4float
%30 = OpLabel
%31 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
%35 = OpLoad %v4float %31
OpStore %29 %35
OpReturn
OpFunctionEnd
%outParameterWriteIndirect_vh4 = OpFunction %void None %27
%36 = OpFunctionParameter %_ptr_Function_v4float
%37 = OpLabel
%38 = OpFunctionCall %void %outParameterWrite_vh4 %36
OpReturn
OpFunctionEnd
%inoutParameterWrite_vh4 = OpFunction %void None %27
%39 = OpFunctionParameter %_ptr_Function_v4float
%40 = OpLabel
%41 = OpLoad %v4float %39
%42 = OpLoad %v4float %39
%43 = OpFMul %v4float %41 %42
OpStore %39 %43
OpReturn
OpFunctionEnd
%inoutParameterWriteIndirect_vh4 = OpFunction %void None %27
%44 = OpFunctionParameter %_ptr_Function_v4float
%45 = OpLabel
%46 = OpFunctionCall %void %inoutParameterWrite_vh4 %44
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %47
%48 = OpFunctionParameter %_ptr_Function_v2float
%49 = OpLabel
%c = OpVariable %_ptr_Function_v4float Function
%51 = OpFunctionCall %void %outParameterWrite_vh4 %c
%52 = OpFunctionCall %void %outParameterWriteIndirect_vh4 %c
%53 = OpFunctionCall %void %inoutParameterWrite_vh4 %c
%54 = OpFunctionCall %void %inoutParameterWriteIndirect_vh4 %c
%56 = OpLoad %v4float %c
OpReturnValue %56
OpFunctionEnd
