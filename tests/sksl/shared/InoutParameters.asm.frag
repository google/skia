OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpName %_entrypoint_v "_entrypoint_v"
OpName %outParameterWrite_vh4 "outParameterWrite_vh4"
OpName %outParameterWriteIndirect_vh4 "outParameterWriteIndirect_vh4"
OpName %inoutParameterWrite_vh4 "inoutParameterWrite_vh4"
OpName %inoutParameterWriteIndirect_vh4 "inoutParameterWriteIndirect_vh4"
OpName %main "main"
OpName %c "c"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %14 Binding 0
OpDecorate %14 DescriptorSet 0
OpDecorate %35 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %c RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%14 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%19 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%23 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%27 = OpTypeFunction %void %_ptr_Function_v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%52 = OpTypeFunction %v4float %_ptr_Function_v2float
%false = OpConstantFalse %bool
%_entrypoint_v = OpFunction %void None %19
%20 = OpLabel
%24 = OpVariable %_ptr_Function_v2float Function
OpStore %24 %23
%26 = OpFunctionCall %v4float %main %24
OpStore %sk_FragColor %26
OpReturn
OpFunctionEnd
%outParameterWrite_vh4 = OpFunction %void None %27
%29 = OpFunctionParameter %_ptr_Function_v4float
%30 = OpLabel
%31 = OpAccessChain %_ptr_Uniform_v4float %14 %int_0
%35 = OpLoad %v4float %31
OpStore %29 %35
OpReturn
OpFunctionEnd
%outParameterWriteIndirect_vh4 = OpFunction %void None %27
%36 = OpFunctionParameter %_ptr_Function_v4float
%37 = OpLabel
%38 = OpVariable %_ptr_Function_v4float Function
%39 = OpFunctionCall %void %outParameterWrite_vh4 %38
%40 = OpLoad %v4float %38
OpStore %36 %40
OpReturn
OpFunctionEnd
%inoutParameterWrite_vh4 = OpFunction %void None %27
%41 = OpFunctionParameter %_ptr_Function_v4float
%42 = OpLabel
%43 = OpLoad %v4float %41
%44 = OpLoad %v4float %41
%45 = OpFMul %v4float %43 %44
OpStore %41 %45
OpReturn
OpFunctionEnd
%inoutParameterWriteIndirect_vh4 = OpFunction %void None %27
%46 = OpFunctionParameter %_ptr_Function_v4float
%47 = OpLabel
%49 = OpVariable %_ptr_Function_v4float Function
%48 = OpLoad %v4float %46
OpStore %49 %48
%50 = OpFunctionCall %void %inoutParameterWrite_vh4 %49
%51 = OpLoad %v4float %49
OpStore %46 %51
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %52
%53 = OpFunctionParameter %_ptr_Function_v2float
%54 = OpLabel
%c = OpVariable %_ptr_Function_v4float Function
%56 = OpVariable %_ptr_Function_v4float Function
%59 = OpVariable %_ptr_Function_v4float Function
%63 = OpVariable %_ptr_Function_v4float Function
%67 = OpVariable %_ptr_Function_v4float Function
%57 = OpFunctionCall %void %outParameterWrite_vh4 %56
%58 = OpLoad %v4float %56
OpStore %c %58
%60 = OpFunctionCall %void %outParameterWriteIndirect_vh4 %59
%61 = OpLoad %v4float %59
OpStore %c %61
%62 = OpLoad %v4float %c
OpStore %63 %62
%64 = OpFunctionCall %void %inoutParameterWrite_vh4 %63
%65 = OpLoad %v4float %63
OpStore %c %65
%66 = OpLoad %v4float %c
OpStore %67 %66
%68 = OpFunctionCall %void %inoutParameterWriteIndirect_vh4 %67
%69 = OpLoad %v4float %67
OpStore %c %69
%71 = OpLoad %v4float %c
OpReturnValue %71
OpFunctionEnd
