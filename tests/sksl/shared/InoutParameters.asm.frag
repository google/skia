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
OpName %outParameterWrite_vh4 "outParameterWrite_vh4"
OpName %outParameterWriteIndirect_vh4 "outParameterWriteIndirect_vh4"
OpName %inoutParameterWrite_vh4 "inoutParameterWrite_vh4"
OpName %inoutParameterWriteIndirect_vh4 "inoutParameterWriteIndirect_vh4"
OpName %inoutParameterRead_vh4 "inoutParameterRead_vh4"
OpName %inoutParameterIgnore_vh4 "inoutParameterIgnore_vh4"
OpName %main "main"
OpName %c "c"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %16 Binding 0
OpDecorate %16 DescriptorSet 0
OpDecorate %37 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %c RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%16 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%21 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%25 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%30 = OpTypeFunction %void %_ptr_Function_v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%58 = OpTypeFunction %v4float %_ptr_Function_v2float
%_entrypoint_v = OpFunction %void None %21
%22 = OpLabel
%26 = OpVariable %_ptr_Function_v2float Function
OpStore %26 %25
%28 = OpFunctionCall %v4float %main %26
OpStore %sk_FragColor %28
OpReturn
OpFunctionEnd
%outParameterWrite_vh4 = OpFunction %void None %30
%31 = OpFunctionParameter %_ptr_Function_v4float
%32 = OpLabel
%33 = OpAccessChain %_ptr_Uniform_v4float %16 %int_0
%37 = OpLoad %v4float %33
OpStore %31 %37
OpReturn
OpFunctionEnd
%outParameterWriteIndirect_vh4 = OpFunction %void None %30
%38 = OpFunctionParameter %_ptr_Function_v4float
%39 = OpLabel
%40 = OpVariable %_ptr_Function_v4float Function
%41 = OpFunctionCall %void %outParameterWrite_vh4 %40
%42 = OpLoad %v4float %40
OpStore %38 %42
OpReturn
OpFunctionEnd
%inoutParameterWrite_vh4 = OpFunction %void None %30
%43 = OpFunctionParameter %_ptr_Function_v4float
%44 = OpLabel
%45 = OpLoad %v4float %43
%46 = OpLoad %v4float %43
%47 = OpFMul %v4float %45 %46
OpStore %43 %47
OpReturn
OpFunctionEnd
%inoutParameterWriteIndirect_vh4 = OpFunction %void None %30
%48 = OpFunctionParameter %_ptr_Function_v4float
%49 = OpLabel
%51 = OpVariable %_ptr_Function_v4float Function
%50 = OpLoad %v4float %48
OpStore %51 %50
%52 = OpFunctionCall %void %inoutParameterWrite_vh4 %51
%53 = OpLoad %v4float %51
OpStore %48 %53
OpReturn
OpFunctionEnd
%inoutParameterRead_vh4 = OpFunction %void None %30
%54 = OpFunctionParameter %_ptr_Function_v4float
%55 = OpLabel
OpReturn
OpFunctionEnd
%inoutParameterIgnore_vh4 = OpFunction %void None %30
%56 = OpFunctionParameter %_ptr_Function_v4float
%57 = OpLabel
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %58
%59 = OpFunctionParameter %_ptr_Function_v2float
%60 = OpLabel
%c = OpVariable %_ptr_Function_v4float Function
%62 = OpVariable %_ptr_Function_v4float Function
%65 = OpVariable %_ptr_Function_v4float Function
%68 = OpVariable %_ptr_Function_v4float Function
%71 = OpVariable %_ptr_Function_v4float Function
%74 = OpVariable %_ptr_Function_v4float Function
%77 = OpVariable %_ptr_Function_v4float Function
%63 = OpFunctionCall %void %outParameterWrite_vh4 %62
%64 = OpLoad %v4float %62
OpStore %c %64
%66 = OpFunctionCall %void %outParameterWriteIndirect_vh4 %65
%67 = OpLoad %v4float %65
OpStore %c %67
OpStore %68 %67
%69 = OpFunctionCall %void %inoutParameterWrite_vh4 %68
%70 = OpLoad %v4float %68
OpStore %c %70
OpStore %71 %70
%72 = OpFunctionCall %void %inoutParameterWriteIndirect_vh4 %71
%73 = OpLoad %v4float %71
OpStore %c %73
OpStore %74 %73
%75 = OpFunctionCall %void %inoutParameterRead_vh4 %74
%76 = OpLoad %v4float %74
OpStore %c %76
OpStore %77 %76
%78 = OpFunctionCall %void %inoutParameterIgnore_vh4 %77
%79 = OpLoad %v4float %77
OpStore %c %79
OpReturnValue %79
OpFunctionEnd
