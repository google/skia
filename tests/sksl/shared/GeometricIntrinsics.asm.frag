OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpName %_entrypoint_v "_entrypoint_v"
OpName %scalar_fff "scalar_fff"
OpName %vector_f2f2f2 "vector_f2f2f2"
OpName %main "main"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %70 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_float = OpTypePointer Function %float
%26 = OpTypeFunction %float %_ptr_Function_float %_ptr_Function_float
%37 = OpTypeFunction %v2float %_ptr_Function_v2float %_ptr_Function_v2float
%51 = OpTypeFunction %v4float %_ptr_Function_v2float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%59 = OpConstantComposite %v2float %float_1 %float_2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%63 = OpConstantComposite %v2float %float_3 %float_4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%22 = OpVariable %_ptr_Function_v2float Function
OpStore %22 %21
%24 = OpFunctionCall %v4float %main %22
OpStore %sk_FragColor %24
OpReturn
OpFunctionEnd
%scalar_fff = OpFunction %float None %26
%27 = OpFunctionParameter %_ptr_Function_float
%28 = OpFunctionParameter %_ptr_Function_float
%29 = OpLabel
%31 = OpLoad %float %27
%30 = OpExtInst %float %1 Length %31
OpStore %27 %30
%33 = OpLoad %float %28
%32 = OpExtInst %float %1 Distance %30 %33
OpStore %27 %32
%35 = OpLoad %float %28
%34 = OpFMul %float %32 %35
OpStore %27 %34
%36 = OpExtInst %float %1 Normalize %34
OpStore %27 %36
OpReturnValue %36
OpFunctionEnd
%vector_f2f2f2 = OpFunction %v2float None %37
%38 = OpFunctionParameter %_ptr_Function_v2float
%39 = OpFunctionParameter %_ptr_Function_v2float
%40 = OpLabel
%42 = OpLoad %v2float %38
%41 = OpExtInst %float %1 Length %42
%43 = OpCompositeConstruct %v2float %41 %41
OpStore %38 %43
%45 = OpLoad %v2float %39
%44 = OpExtInst %float %1 Distance %43 %45
%46 = OpCompositeConstruct %v2float %44 %44
OpStore %38 %46
%48 = OpLoad %v2float %39
%47 = OpDot %float %46 %48
%49 = OpCompositeConstruct %v2float %47 %47
OpStore %38 %49
%50 = OpExtInst %v2float %1 Normalize %49
OpStore %38 %50
OpReturnValue %50
OpFunctionEnd
%main = OpFunction %v4float None %51
%52 = OpFunctionParameter %_ptr_Function_v2float
%53 = OpLabel
%55 = OpVariable %_ptr_Function_float Function
%57 = OpVariable %_ptr_Function_float Function
%60 = OpVariable %_ptr_Function_v2float Function
%64 = OpVariable %_ptr_Function_v2float Function
OpStore %55 %float_1
OpStore %57 %float_2
%58 = OpFunctionCall %float %scalar_fff %55 %57
OpStore %60 %59
OpStore %64 %63
%65 = OpFunctionCall %v2float %vector_f2f2f2 %60 %64
%66 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%70 = OpLoad %v4float %66
OpReturnValue %70
OpFunctionEnd
