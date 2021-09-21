OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %switch_fallthrough_bi "switch_fallthrough_bi"
OpName %switch_fallthrough_twice_bi "switch_fallthrough_twice_bi"
OpName %main "main"
OpName %x "x"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%26 = OpTypeFunction %bool %_ptr_Function_int
%false = OpConstantFalse %bool
%true = OpConstantTrue %bool
%47 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%22 = OpVariable %_ptr_Function_v2float Function
OpStore %22 %21
%24 = OpFunctionCall %v4float %main %22
OpStore %sk_FragColor %24
OpReturn
OpFunctionEnd
%switch_fallthrough_bi = OpFunction %bool None %26
%28 = OpFunctionParameter %_ptr_Function_int
%29 = OpLabel
%30 = OpLoad %int %28
OpSelectionMerge %31 None
OpSwitch %30 %35 2 %32 1 %33 0 %34
%32 = OpLabel
OpReturnValue %false
%33 = OpLabel
OpBranch %34
%34 = OpLabel
OpReturnValue %true
%35 = OpLabel
OpReturnValue %false
%31 = OpLabel
OpUnreachable
OpFunctionEnd
%switch_fallthrough_twice_bi = OpFunction %bool None %26
%38 = OpFunctionParameter %_ptr_Function_int
%39 = OpLabel
%40 = OpLoad %int %38
OpSelectionMerge %41 None
OpSwitch %40 %46 0 %42 1 %43 2 %44 3 %45
%42 = OpLabel
OpReturnValue %false
%43 = OpLabel
OpBranch %44
%44 = OpLabel
OpBranch %45
%45 = OpLabel
OpReturnValue %true
%46 = OpLabel
OpReturnValue %false
%41 = OpLabel
OpUnreachable
OpFunctionEnd
%main = OpFunction %v4float None %47
%48 = OpFunctionParameter %_ptr_Function_v2float
%49 = OpLabel
%x = OpVariable %_ptr_Function_int Function
%58 = OpVariable %_ptr_Function_int Function
%63 = OpVariable %_ptr_Function_int Function
%66 = OpVariable %_ptr_Function_v4float Function
%51 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%54 = OpLoad %v4float %51
%55 = OpCompositeExtract %float %54 1
%56 = OpConvertFToS %int %55
OpStore %x %56
%57 = OpLoad %int %x
OpStore %58 %57
%59 = OpFunctionCall %bool %switch_fallthrough_bi %58
OpSelectionMerge %61 None
OpBranchConditional %59 %60 %61
%60 = OpLabel
%62 = OpLoad %int %x
OpStore %63 %62
%64 = OpFunctionCall %bool %switch_fallthrough_twice_bi %63
OpBranch %61
%61 = OpLabel
%65 = OpPhi %bool %false %49 %64 %60
OpSelectionMerge %70 None
OpBranchConditional %65 %68 %69
%68 = OpLabel
%71 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%72 = OpLoad %v4float %71
OpStore %66 %72
OpBranch %70
%69 = OpLabel
%73 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%75 = OpLoad %v4float %73
OpStore %66 %75
OpBranch %70
%70 = OpLabel
%76 = OpLoad %v4float %66
OpReturnValue %76
OpFunctionEnd
