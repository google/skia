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
OpName %_1_m24 "_1_m24"
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
OpDecorate %37 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
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
%mat2v4float = OpTypeMatrix %v4float 2
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
%float_3 = OpConstant %float 3
%false = OpConstantFalse %bool
%v4bool = OpTypeVector %bool 4
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
%_1_m24 = OpVariable %_ptr_Function_mat2v4float Function
%56 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%34 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%35 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%33 = OpCompositeConstruct %mat2v4float %34 %35
OpStore %_1_m24 %33
%37 = OpLoad %bool %_0_ok
OpSelectionMerge %39 None
OpBranchConditional %37 %38 %39
%38 = OpLabel
%40 = OpLoad %mat2v4float %_1_m24
%42 = OpCompositeConstruct %v4float %float_3 %float_0 %float_0 %float_0
%43 = OpCompositeConstruct %v4float %float_0 %float_3 %float_0 %float_0
%41 = OpCompositeConstruct %mat2v4float %42 %43
%45 = OpCompositeExtract %v4float %40 0
%46 = OpCompositeExtract %v4float %41 0
%47 = OpFOrdEqual %v4bool %45 %46
%48 = OpAll %bool %47
%49 = OpCompositeExtract %v4float %40 1
%50 = OpCompositeExtract %v4float %41 1
%51 = OpFOrdEqual %v4bool %49 %50
%52 = OpAll %bool %51
%53 = OpLogicalAnd %bool %48 %52
OpBranch %39
%39 = OpLabel
%54 = OpPhi %bool %false %25 %53 %38
OpStore %_0_ok %54
%55 = OpLoad %bool %_0_ok
OpSelectionMerge %60 None
OpBranchConditional %55 %58 %59
%58 = OpLabel
%61 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%65 = OpLoad %v4float %61
OpStore %56 %65
OpBranch %60
%59 = OpLabel
%66 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%68 = OpLoad %v4float %66
OpStore %56 %68
OpBranch %60
%60 = OpLabel
%69 = OpLoad %v4float %56
OpReturnValue %69
OpFunctionEnd
