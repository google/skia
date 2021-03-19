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
OpMemberName %_UniformBuffer 2 "c"
OpMemberName %_UniformBuffer 3 "d"
OpMemberName %_UniformBuffer 4 "e"
OpMemberName %_UniformBuffer 5 "f"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 3 Offset 40
OpMemberDecorate %_UniformBuffer 4 Offset 48
OpMemberDecorate %_UniformBuffer 5 Offset 64
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %24 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%uint = OpTypeInt 32 0
%v2uint = OpTypeVector %uint 2
%int = OpTypeInt 32 1
%v3int = OpTypeVector %int 3
%_UniformBuffer = OpTypeStruct %v4float %v4float %v2uint %v2uint %v3int %v3int
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%18 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%v4bool = OpTypeVector %bool 4
%_ptr_Output_float = OpTypePointer Output %float
%_ptr_Uniform_v2uint = OpTypePointer Uniform %v2uint
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%v2bool = OpTypeVector %bool 2
%_ptr_Uniform_v3int = OpTypePointer Uniform %v3int
%int_4 = OpConstant %int 4
%int_5 = OpConstant %int 5
%v3bool = OpTypeVector %bool 3
%main = OpFunction %void None %18
%19 = OpLabel
%21 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%24 = OpLoad %v4float %21
%25 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%27 = OpLoad %v4float %25
%20 = OpFOrdLessThan %v4bool %24 %27
%29 = OpCompositeExtract %bool %20 0
%30 = OpSelect %int %29 %int_1 %int_0
%31 = OpConvertSToF %float %30
%32 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %32 %31
%35 = OpAccessChain %_ptr_Uniform_v2uint %10 %int_2
%38 = OpLoad %v2uint %35
%39 = OpAccessChain %_ptr_Uniform_v2uint %10 %int_3
%41 = OpLoad %v2uint %39
%34 = OpULessThan %v2bool %38 %41
%43 = OpCompositeExtract %bool %34 1
%44 = OpSelect %int %43 %int_1 %int_0
%45 = OpConvertSToF %float %44
%46 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_1
OpStore %46 %45
%48 = OpAccessChain %_ptr_Uniform_v3int %10 %int_4
%51 = OpLoad %v3int %48
%52 = OpAccessChain %_ptr_Uniform_v3int %10 %int_5
%54 = OpLoad %v3int %52
%47 = OpSLessThan %v3bool %51 %54
%56 = OpCompositeExtract %bool %47 2
%57 = OpSelect %int %56 %int_1 %int_0
%58 = OpConvertSToF %float %57
%59 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_2
OpStore %59 %58
OpReturn
OpFunctionEnd
