OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "a"
OpMemberName %_UniformBuffer 1 "b"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 1 Offset 4
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%int = OpTypeInt 32 1
%uint = OpTypeInt 32 0
%_UniformBuffer = OpTypeStruct %int %uint
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%16 = OpTypeFunction %void
%_ptr_Uniform_int = OpTypePointer Uniform %int
%int_0 = OpConstant %int 0
%_ptr_Output_float = OpTypePointer Output %float
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
%int_1 = OpConstant %int 1
%main = OpFunction %void None %16
%17 = OpLabel
%19 = OpAccessChain %_ptr_Uniform_int %10 %int_0
%22 = OpLoad %int %19
%18 = OpExtInst %int %1 FindSMsb %22
%23 = OpConvertSToF %float %18
%24 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %24 %23
%27 = OpAccessChain %_ptr_Uniform_uint %10 %int_1
%30 = OpLoad %uint %27
%26 = OpExtInst %int %1 FindUMsb %30
%31 = OpConvertSToF %float %26
%32 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_1
OpStore %32 %31
OpReturn
OpFunctionEnd
