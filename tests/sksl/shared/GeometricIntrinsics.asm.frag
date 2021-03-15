OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpName %_entrypoint "_entrypoint"
OpName %scalar "scalar"
OpName %vector "vector"
OpName %main "main"
OpName %x "x"
OpName %y "y"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %76 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%20 = OpTypeFunction %float %_ptr_Function_float %_ptr_Function_float
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%37 = OpTypeFunction %v2float %_ptr_Function_v2float %_ptr_Function_v2float
%56 = OpTypeFunction %v4float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%65 = OpConstantComposite %v2float %float_1 %float_2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%69 = OpConstantComposite %v2float %float_3 %float_4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_entrypoint = OpFunction %void None %17
%18 = OpLabel
%19 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %19
OpReturn
OpFunctionEnd
%scalar = OpFunction %float None %20
%22 = OpFunctionParameter %_ptr_Function_float
%23 = OpFunctionParameter %_ptr_Function_float
%24 = OpLabel
%26 = OpLoad %float %22
%25 = OpExtInst %float %1 Length %26
OpStore %22 %25
%28 = OpLoad %float %22
%29 = OpLoad %float %23
%27 = OpExtInst %float %1 Distance %28 %29
OpStore %22 %27
%31 = OpLoad %float %22
%32 = OpLoad %float %23
%30 = OpFMul %float %31 %32
OpStore %22 %30
%34 = OpLoad %float %22
%33 = OpExtInst %float %1 Normalize %34
OpStore %22 %33
%35 = OpLoad %float %22
OpReturnValue %35
OpFunctionEnd
%vector = OpFunction %v2float None %37
%39 = OpFunctionParameter %_ptr_Function_v2float
%40 = OpFunctionParameter %_ptr_Function_v2float
%41 = OpLabel
%43 = OpLoad %v2float %39
%42 = OpExtInst %float %1 Length %43
%44 = OpCompositeConstruct %v2float %42 %42
OpStore %39 %44
%46 = OpLoad %v2float %39
%47 = OpLoad %v2float %40
%45 = OpExtInst %float %1 Distance %46 %47
%48 = OpCompositeConstruct %v2float %45 %45
OpStore %39 %48
%50 = OpLoad %v2float %39
%51 = OpLoad %v2float %40
%49 = OpDot %float %50 %51
%52 = OpCompositeConstruct %v2float %49 %49
OpStore %39 %52
%54 = OpLoad %v2float %39
%53 = OpExtInst %v2float %1 Normalize %54
OpStore %39 %53
%55 = OpLoad %v2float %39
OpReturnValue %55
OpFunctionEnd
%main = OpFunction %v4float None %56
%57 = OpLabel
%x = OpVariable %_ptr_Function_float Function
%60 = OpVariable %_ptr_Function_float Function
%62 = OpVariable %_ptr_Function_float Function
%y = OpVariable %_ptr_Function_v2float Function
%66 = OpVariable %_ptr_Function_v2float Function
%70 = OpVariable %_ptr_Function_v2float Function
OpStore %60 %float_1
OpStore %62 %float_2
%63 = OpFunctionCall %float %scalar %60 %62
OpStore %x %63
OpStore %66 %65
OpStore %70 %69
%71 = OpFunctionCall %v2float %vector %66 %70
OpStore %y %71
%72 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%76 = OpLoad %v4float %72
OpReturnValue %76
OpFunctionEnd
