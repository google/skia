OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint "_entrypoint"
OpName %main "main"
OpName %TRUE "TRUE"
OpName %FALSE "FALSE"
OpName %ok "ok"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %26 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
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
%18 = OpTypeFunction %v4float
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_1 = OpConstant %float 1
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int_1 = OpConstant %int 1
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%TRUE = OpVariable %_ptr_Function_bool Function
%FALSE = OpVariable %_ptr_Function_bool Function
%ok = OpVariable %_ptr_Function_bool Function
%49 = OpVariable %_ptr_Function_v4float Function
OpStore %TRUE %true
OpStore %FALSE %false
OpStore %ok %true
%26 = OpLoad %bool %ok
OpSelectionMerge %28 None
OpBranchConditional %26 %27 %28
%27 = OpLabel
%29 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%33 = OpLoad %v4float %29
%34 = OpCompositeExtract %float %33 1
%36 = OpFOrdEqual %bool %34 %float_1
%37 = OpSelect %bool %36 %true %false
OpBranch %28
%28 = OpLabel
%38 = OpPhi %bool %false %19 %37 %27
OpStore %ok %38
%39 = OpLoad %bool %ok
OpSelectionMerge %41 None
OpBranchConditional %39 %40 %41
%40 = OpLabel
%42 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%43 = OpLoad %v4float %42
%44 = OpCompositeExtract %float %43 0
%45 = OpFOrdEqual %bool %44 %float_1
%46 = OpSelect %bool %45 %false %true
OpBranch %41
%41 = OpLabel
%47 = OpPhi %bool %false %28 %46 %40
OpStore %ok %47
%48 = OpLoad %bool %ok
OpSelectionMerge %53 None
OpBranchConditional %48 %51 %52
%51 = OpLabel
%54 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%55 = OpLoad %v4float %54
OpStore %49 %55
OpBranch %53
%52 = OpLabel
%56 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%58 = OpLoad %v4float %56
OpStore %49 %58
OpBranch %53
%53 = OpLabel
%59 = OpLoad %v4float %49
OpReturnValue %59
OpFunctionEnd
