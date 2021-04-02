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
OpDecorate %26 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
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
%24 = OpCompositeExtract %float %17 0
%25 = OpCompositeExtract %float %17 1
%26 = OpCompositeConstruct %v2float %24 %25
%27 = OpLoad %v4float %sk_FragColor
%28 = OpVectorShuffle %v4float %27 %26 4 5 2 3
OpStore %sk_FragColor %28
%30 = OpAccessChain %_ptr_Uniform_uint %10 %int_0
%31 = OpLoad %uint %30
%29 = OpExtInst %v2float %1 UnpackUnorm2x16 %31
%32 = OpCompositeExtract %float %29 0
%33 = OpCompositeExtract %float %29 1
%34 = OpCompositeConstruct %v2float %32 %33
%35 = OpLoad %v4float %sk_FragColor
%36 = OpVectorShuffle %v4float %35 %34 4 5 2 3
OpStore %sk_FragColor %36
%38 = OpAccessChain %_ptr_Uniform_uint %10 %int_0
%39 = OpLoad %uint %38
%37 = OpExtInst %v2float %1 UnpackSnorm2x16 %39
%40 = OpCompositeExtract %float %37 0
%41 = OpCompositeExtract %float %37 1
%42 = OpCompositeConstruct %v2float %40 %41
%43 = OpLoad %v4float %sk_FragColor
%44 = OpVectorShuffle %v4float %43 %42 4 5 2 3
OpStore %sk_FragColor %44
%46 = OpAccessChain %_ptr_Uniform_uint %10 %int_0
%47 = OpLoad %uint %46
%45 = OpExtInst %v4float %1 UnpackUnorm4x8 %47
%48 = OpCompositeExtract %float %45 0
%49 = OpCompositeExtract %float %45 1
%50 = OpCompositeExtract %float %45 2
%51 = OpCompositeExtract %float %45 3
%52 = OpCompositeConstruct %v4float %48 %49 %50 %51
OpStore %sk_FragColor %52
%54 = OpAccessChain %_ptr_Uniform_uint %10 %int_0
%55 = OpLoad %uint %54
%53 = OpExtInst %v4float %1 UnpackSnorm4x8 %55
%56 = OpCompositeExtract %float %53 0
%57 = OpCompositeExtract %float %53 1
%58 = OpCompositeExtract %float %53 2
%59 = OpCompositeExtract %float %53 3
%60 = OpCompositeConstruct %v4float %56 %57 %58 %59
OpStore %sk_FragColor %60
OpReturn
OpFunctionEnd
