OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "unknownInput"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %x "x"
OpName %v "v"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %x RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %v RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Uniform_float = OpTypePointer Uniform %float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_1 = OpConstant %float 1
%_entrypoint_v = OpFunction %void None %15
%16 = OpLabel
%20 = OpVariable %_ptr_Function_v2float Function
OpStore %20 %19
%22 = OpFunctionCall %v4float %main %20
OpStore %sk_FragColor %22
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %23
%24 = OpFunctionParameter %_ptr_Function_v2float
%25 = OpLabel
%x = OpVariable %_ptr_Function_float Function
%v = OpVariable %_ptr_Function_v4float Function
%28 = OpAccessChain %_ptr_Uniform_float %10 %int_0
%32 = OpLoad %float %28
OpStore %x %32
%35 = OpLoad %float %x
%36 = OpCompositeConstruct %v2float %35 %35
%37 = OpCompositeExtract %float %36 0
%38 = OpCompositeExtract %float %36 1
%40 = OpCompositeConstruct %v4float %37 %38 %float_0 %float_1
OpStore %v %40
%41 = OpAccessChain %_ptr_Uniform_float %10 %int_0
%42 = OpLoad %float %41
%43 = OpCompositeConstruct %v2float %42 %42
%44 = OpCompositeExtract %float %43 0
%45 = OpCompositeExtract %float %43 1
%46 = OpCompositeConstruct %v4float %44 %45 %float_0 %float_1
OpStore %v %46
%47 = OpAccessChain %_ptr_Uniform_float %10 %int_0
%48 = OpLoad %float %47
%49 = OpCompositeConstruct %v4float %float_0 %48 %float_1 %float_0
OpStore %v %49
%50 = OpAccessChain %_ptr_Uniform_float %10 %int_0
%51 = OpLoad %float %50
%52 = OpAccessChain %_ptr_Uniform_float %10 %int_0
%53 = OpLoad %float %52
%54 = OpCompositeConstruct %v4float %float_0 %51 %float_0 %53
OpStore %v %54
%55 = OpLoad %v4float %v
OpReturnValue %55
OpFunctionEnd
