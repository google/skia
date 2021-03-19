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
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %22 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%_UniformBuffer = OpTypeStruct %v2float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%uint = OpTypeInt 32 0
%_ptr_Output_float = OpTypePointer Output %float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_1 = OpConstant %int 1
%main = OpFunction %void None %15
%16 = OpLabel
%18 = OpAccessChain %_ptr_Uniform_v2float %10 %int_0
%22 = OpLoad %v2float %18
%17 = OpExtInst %uint %1 PackHalf2x16 %22
%24 = OpConvertUToF %float %17
%25 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %25 %24
%28 = OpAccessChain %_ptr_Uniform_v2float %10 %int_0
%29 = OpLoad %v2float %28
%27 = OpExtInst %uint %1 PackUnorm2x16 %29
%30 = OpConvertUToF %float %27
%31 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %31 %30
%33 = OpAccessChain %_ptr_Uniform_v2float %10 %int_0
%34 = OpLoad %v2float %33
%32 = OpExtInst %uint %1 PackSnorm2x16 %34
%35 = OpConvertUToF %float %32
%36 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %36 %35
%38 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%41 = OpLoad %v4float %38
%37 = OpExtInst %uint %1 PackUnorm4x8 %41
%42 = OpConvertUToF %float %37
%43 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %43 %42
%45 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%46 = OpLoad %v4float %45
%44 = OpExtInst %uint %1 PackSnorm4x8 %46
%47 = OpConvertUToF %float %44
%48 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %48 %47
OpReturn
OpFunctionEnd
