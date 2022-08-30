OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %switch_fallthrough_bi "switch_fallthrough_bi"
OpName %ok "ok"
OpName %switch_fallthrough_twice_bi "switch_fallthrough_twice_bi"
OpName %ok_0 "ok"
OpName %main "main"
OpName %x "x"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %40 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%27 = OpTypeFunction %bool %_ptr_Function_int
%_ptr_Function_bool = OpTypePointer Function %bool
%false = OpConstantFalse %bool
%true = OpConstantTrue %bool
%52 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%switch_fallthrough_bi = OpFunction %bool None %27
%28 = OpFunctionParameter %_ptr_Function_int
%29 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
OpStore %ok %false
%33 = OpLoad %int %28
OpSelectionMerge %34 None
OpSwitch %33 %38 2 %35 1 %36 0 %37
%35 = OpLabel
OpBranch %34
%36 = OpLabel
OpBranch %37
%37 = OpLabel
OpStore %ok %true
OpBranch %34
%38 = OpLabel
OpBranch %34
%34 = OpLabel
%40 = OpLoad %bool %ok
OpReturnValue %40
OpFunctionEnd
%switch_fallthrough_twice_bi = OpFunction %bool None %27
%41 = OpFunctionParameter %_ptr_Function_int
%42 = OpLabel
%ok_0 = OpVariable %_ptr_Function_bool Function
OpStore %ok_0 %false
%44 = OpLoad %int %41
OpSelectionMerge %45 None
OpSwitch %44 %50 0 %46 1 %47 2 %48 3 %49
%46 = OpLabel
OpBranch %45
%47 = OpLabel
OpBranch %48
%48 = OpLabel
OpBranch %49
%49 = OpLabel
OpStore %ok_0 %true
OpBranch %45
%50 = OpLabel
OpBranch %45
%45 = OpLabel
%51 = OpLoad %bool %ok_0
OpReturnValue %51
OpFunctionEnd
%main = OpFunction %v4float None %52
%53 = OpFunctionParameter %_ptr_Function_v2float
%54 = OpLabel
%x = OpVariable %_ptr_Function_int Function
%62 = OpVariable %_ptr_Function_int Function
%66 = OpVariable %_ptr_Function_int Function
%69 = OpVariable %_ptr_Function_v4float Function
%56 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%59 = OpLoad %v4float %56
%60 = OpCompositeExtract %float %59 1
%61 = OpConvertFToS %int %60
OpStore %x %61
OpStore %62 %61
%63 = OpFunctionCall %bool %switch_fallthrough_bi %62
OpSelectionMerge %65 None
OpBranchConditional %63 %64 %65
%64 = OpLabel
OpStore %66 %61
%67 = OpFunctionCall %bool %switch_fallthrough_twice_bi %66
OpBranch %65
%65 = OpLabel
%68 = OpPhi %bool %false %54 %67 %64
OpSelectionMerge %73 None
OpBranchConditional %68 %71 %72
%71 = OpLabel
%74 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%75 = OpLoad %v4float %74
OpStore %69 %75
OpBranch %73
%72 = OpLabel
%76 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%78 = OpLoad %v4float %76
OpStore %69 %78
OpBranch %73
%73 = OpLabel
%79 = OpLoad %v4float %69
OpReturnValue %79
OpFunctionEnd
