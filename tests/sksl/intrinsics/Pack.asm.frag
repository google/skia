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
OpDecorate %23 RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
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
%23 = OpCompositeExtract %float %22 0
%24 = OpCompositeExtract %float %22 1
%25 = OpCompositeConstruct %v2float %23 %24
%17 = OpExtInst %uint %1 PackHalf2x16 %25
%27 = OpConvertUToF %float %17
%28 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %28 %27
%31 = OpAccessChain %_ptr_Uniform_v2float %10 %int_0
%32 = OpLoad %v2float %31
%33 = OpCompositeExtract %float %32 0
%34 = OpCompositeExtract %float %32 1
%35 = OpCompositeConstruct %v2float %33 %34
%30 = OpExtInst %uint %1 PackUnorm2x16 %35
%36 = OpConvertUToF %float %30
%37 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %37 %36
%39 = OpAccessChain %_ptr_Uniform_v2float %10 %int_0
%40 = OpLoad %v2float %39
%41 = OpCompositeExtract %float %40 0
%42 = OpCompositeExtract %float %40 1
%43 = OpCompositeConstruct %v2float %41 %42
%38 = OpExtInst %uint %1 PackSnorm2x16 %43
%44 = OpConvertUToF %float %38
%45 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %45 %44
%47 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%50 = OpLoad %v4float %47
%51 = OpCompositeExtract %float %50 0
%52 = OpCompositeExtract %float %50 1
%53 = OpCompositeExtract %float %50 2
%54 = OpCompositeExtract %float %50 3
%55 = OpCompositeConstruct %v4float %51 %52 %53 %54
%46 = OpExtInst %uint %1 PackUnorm4x8 %55
%56 = OpConvertUToF %float %46
%57 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %57 %56
%59 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%60 = OpLoad %v4float %59
%61 = OpCompositeExtract %float %60 0
%62 = OpCompositeExtract %float %60 1
%63 = OpCompositeExtract %float %60 2
%64 = OpCompositeExtract %float %60 3
%65 = OpCompositeConstruct %v4float %61 %62 %63 %64
%58 = OpExtInst %uint %1 PackSnorm4x8 %65
%66 = OpConvertUToF %float %58
%67 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %67 %66
OpReturn
OpFunctionEnd
