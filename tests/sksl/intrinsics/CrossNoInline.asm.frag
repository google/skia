OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "ah"
OpMemberName %_UniformBuffer 1 "bh"
OpMemberName %_UniformBuffer 2 "af"
OpMemberName %_UniformBuffer 3 "bf"
OpName %cross_length_2d_hh2h2 "cross_length_2d_hh2h2"
OpName %cross_length_2d_ff2f2 "cross_length_2d_ff2f2"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 8
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 16
OpMemberDecorate %_UniformBuffer 3 Offset 24
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %21 RelaxedPrecision
OpDecorate %22 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%_UniformBuffer = OpTypeStruct %v2float %v2float %v2float %v2float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%_ptr_Function_v2float = OpTypePointer Function %v2float
%16 = OpTypeFunction %float %_ptr_Function_v2float %_ptr_Function_v2float
%void = OpTypeVoid
%47 = OpTypeFunction %void
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_ptr_Output_float = OpTypePointer Output %float
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%cross_length_2d_hh2h2 = OpFunction %float None %16
%18 = OpFunctionParameter %_ptr_Function_v2float
%19 = OpFunctionParameter %_ptr_Function_v2float
%20 = OpLabel
%21 = OpLoad %v2float %18
%22 = OpCompositeExtract %float %21 0
%23 = OpLoad %v2float %19
%24 = OpCompositeExtract %float %23 1
%25 = OpFMul %float %22 %24
%26 = OpLoad %v2float %18
%27 = OpCompositeExtract %float %26 1
%28 = OpLoad %v2float %19
%29 = OpCompositeExtract %float %28 0
%30 = OpFMul %float %27 %29
%31 = OpFSub %float %25 %30
OpReturnValue %31
OpFunctionEnd
%cross_length_2d_ff2f2 = OpFunction %float None %16
%32 = OpFunctionParameter %_ptr_Function_v2float
%33 = OpFunctionParameter %_ptr_Function_v2float
%34 = OpLabel
%35 = OpLoad %v2float %32
%36 = OpCompositeExtract %float %35 0
%37 = OpLoad %v2float %33
%38 = OpCompositeExtract %float %37 1
%39 = OpFMul %float %36 %38
%40 = OpLoad %v2float %32
%41 = OpCompositeExtract %float %40 1
%42 = OpLoad %v2float %33
%43 = OpCompositeExtract %float %42 0
%44 = OpFMul %float %41 %43
%45 = OpFSub %float %39 %44
OpReturnValue %45
OpFunctionEnd
%main = OpFunction %void None %47
%48 = OpLabel
%54 = OpVariable %_ptr_Function_v2float Function
%58 = OpVariable %_ptr_Function_v2float Function
%65 = OpVariable %_ptr_Function_v2float Function
%69 = OpVariable %_ptr_Function_v2float Function
%49 = OpAccessChain %_ptr_Uniform_v2float %12 %int_0
%53 = OpLoad %v2float %49
OpStore %54 %53
%55 = OpAccessChain %_ptr_Uniform_v2float %12 %int_1
%57 = OpLoad %v2float %55
OpStore %58 %57
%59 = OpFunctionCall %float %cross_length_2d_hh2h2 %54 %58
%60 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %60 %59
%62 = OpAccessChain %_ptr_Uniform_v2float %12 %int_2
%64 = OpLoad %v2float %62
OpStore %65 %64
%66 = OpAccessChain %_ptr_Uniform_v2float %12 %int_3
%68 = OpLoad %v2float %66
OpStore %69 %68
%70 = OpFunctionCall %float %cross_length_2d_ff2f2 %65 %69
%71 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_1
OpStore %71 %70
OpReturn
OpFunctionEnd
