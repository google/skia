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
OpName %switch_fallthrough_twice_bi "switch_fallthrough_twice_bi"
OpName %ok "ok"
OpName %main "main"
OpName %x "x"
OpName %_0_ok "_0_ok"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %40 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%16 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%26 = OpTypeFunction %bool %_ptr_Function_int
%_ptr_Function_bool = OpTypePointer Function %bool
%false = OpConstantFalse %bool
%true = OpConstantTrue %bool
%41 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %16
%17 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%switch_fallthrough_twice_bi = OpFunction %bool None %26
%27 = OpFunctionParameter %_ptr_Function_int
%28 = OpLabel
%ok = OpVariable %_ptr_Function_bool Function
OpStore %ok %false
%32 = OpLoad %int %27
OpSelectionMerge %33 None
OpSwitch %32 %38 0 %34 1 %35 2 %36 3 %37
%34 = OpLabel
OpBranch %33
%35 = OpLabel
OpBranch %36
%36 = OpLabel
OpBranch %37
%37 = OpLabel
OpStore %ok %true
OpBranch %33
%38 = OpLabel
OpBranch %33
%33 = OpLabel
%40 = OpLoad %bool %ok
OpReturnValue %40
OpFunctionEnd
%main = OpFunction %v4float None %41
%42 = OpFunctionParameter %_ptr_Function_v2float
%43 = OpLabel
%x = OpVariable %_ptr_Function_int Function
%_0_ok = OpVariable %_ptr_Function_bool Function
%60 = OpVariable %_ptr_Function_int Function
%63 = OpVariable %_ptr_Function_v4float Function
%45 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%48 = OpLoad %v4float %45
%49 = OpCompositeExtract %float %48 1
%50 = OpConvertFToS %int %49
OpStore %x %50
OpStore %_0_ok %false
OpSelectionMerge %52 None
OpSwitch %50 %56 2 %53 1 %54 0 %55
%53 = OpLabel
OpBranch %52
%54 = OpLabel
OpBranch %55
%55 = OpLabel
OpStore %_0_ok %true
OpBranch %52
%56 = OpLabel
OpBranch %52
%52 = OpLabel
%57 = OpLoad %bool %_0_ok
OpSelectionMerge %59 None
OpBranchConditional %57 %58 %59
%58 = OpLabel
OpStore %60 %50
%61 = OpFunctionCall %bool %switch_fallthrough_twice_bi %60
OpBranch %59
%59 = OpLabel
%62 = OpPhi %bool %false %52 %61 %58
OpSelectionMerge %67 None
OpBranchConditional %62 %65 %66
%65 = OpLabel
%68 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%69 = OpLoad %v4float %68
OpStore %63 %69
OpBranch %67
%66 = OpLabel
%70 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%72 = OpLoad %v4float %70
OpStore %63 %72
OpBranch %67
%67 = OpLabel
%73 = OpLoad %v4float %63
OpReturnValue %73
OpFunctionEnd
