OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "a"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %25 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%uint = OpTypeInt 32 0
%_UniformBuffer = OpTypeStruct %uint
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v2float = OpTypeVector %float 2
%main = OpFunction %void None %15
%16 = OpLabel
%18 = OpAccessChain %_ptr_Uniform_uint %10 %int_0
%22 = OpLoad %uint %18
%17 = OpExtInst %v2float %1 UnpackHalf2x16 %22
%24 = OpLoad %v4float %sk_FragColor
%25 = OpVectorShuffle %v4float %24 %17 4 5 2 3
OpStore %sk_FragColor %25
%27 = OpAccessChain %_ptr_Uniform_uint %10 %int_0
%28 = OpLoad %uint %27
%26 = OpExtInst %v2float %1 UnpackUnorm2x16 %28
%29 = OpLoad %v4float %sk_FragColor
%30 = OpVectorShuffle %v4float %29 %26 4 5 2 3
OpStore %sk_FragColor %30
%32 = OpAccessChain %_ptr_Uniform_uint %10 %int_0
%33 = OpLoad %uint %32
%31 = OpExtInst %v2float %1 UnpackSnorm2x16 %33
%34 = OpLoad %v4float %sk_FragColor
%35 = OpVectorShuffle %v4float %34 %31 4 5 2 3
OpStore %sk_FragColor %35
%37 = OpAccessChain %_ptr_Uniform_uint %10 %int_0
%38 = OpLoad %uint %37
%36 = OpExtInst %v4float %1 UnpackUnorm4x8 %38
OpStore %sk_FragColor %36
%40 = OpAccessChain %_ptr_Uniform_uint %10 %int_0
%41 = OpLoad %uint %40
%39 = OpExtInst %v4float %1 UnpackSnorm4x8 %41
OpStore %sk_FragColor %39
OpReturn
OpFunctionEnd
