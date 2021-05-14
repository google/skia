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
OpName %main "main"
OpName %_0_ok "_0_ok"
OpName %_1_m32 "_1_m32"
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
OpDecorate %38 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
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
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%mat3v2float = OpTypeMatrix %v2float 3
%_ptr_Function_mat3v2float = OpTypePointer Function %mat3v2float
%float_4 = OpConstant %float 4
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
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
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_m32 = OpVariable %_ptr_Function_mat3v2float Function
%63 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%34 = OpCompositeConstruct %v2float %float_4 %float_0
%35 = OpCompositeConstruct %v2float %float_0 %float_4
%36 = OpCompositeConstruct %v2float %float_0 %float_0
%33 = OpCompositeConstruct %mat3v2float %34 %35 %36
OpStore %_1_m32 %33
%38 = OpLoad %bool %_0_ok
OpSelectionMerge %40 None
OpBranchConditional %38 %39 %40
%39 = OpLabel
%41 = OpLoad %mat3v2float %_1_m32
%43 = OpCompositeConstruct %v2float %float_4 %float_0
%44 = OpCompositeConstruct %v2float %float_0 %float_4
%45 = OpCompositeConstruct %v2float %float_0 %float_0
%42 = OpCompositeConstruct %mat3v2float %43 %44 %45
%47 = OpCompositeExtract %v2float %41 0
%48 = OpCompositeExtract %v2float %42 0
%49 = OpFOrdEqual %v2bool %47 %48
%50 = OpAll %bool %49
%51 = OpCompositeExtract %v2float %41 1
%52 = OpCompositeExtract %v2float %42 1
%53 = OpFOrdEqual %v2bool %51 %52
%54 = OpAll %bool %53
%55 = OpLogicalAnd %bool %50 %54
%56 = OpCompositeExtract %v2float %41 2
%57 = OpCompositeExtract %v2float %42 2
%58 = OpFOrdEqual %v2bool %56 %57
%59 = OpAll %bool %58
%60 = OpLogicalAnd %bool %55 %59
OpBranch %40
%40 = OpLabel
%61 = OpPhi %bool %false %25 %60 %39
OpStore %_0_ok %61
%62 = OpLoad %bool %_0_ok
OpSelectionMerge %67 None
OpBranchConditional %62 %65 %66
%65 = OpLabel
%68 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%72 = OpLoad %v4float %68
OpStore %63 %72
OpBranch %67
%66 = OpLabel
%73 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%75 = OpLoad %v4float %73
OpStore %63 %75
OpBranch %67
%67 = OpLabel
%76 = OpLoad %v4float %63
OpReturnValue %76
OpFunctionEnd
