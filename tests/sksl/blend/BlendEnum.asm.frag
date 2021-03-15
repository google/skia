OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "src"
OpMemberName %_UniformBuffer 1 "dst"
OpName %main "main"
OpName %_0_blend "_0_blend"
OpName %_1_loop "_1_loop"
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
OpDecorate %32 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
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
%14 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%main = OpFunction %void None %14
%15 = OpLabel
%_0_blend = OpVariable %_ptr_Function_v4float Function
%_1_loop = OpVariable %_ptr_Function_int Function
OpStore %_1_loop %int_0
OpBranch %22
%22 = OpLabel
OpLoopMerge %26 %25 None
OpBranch %23
%23 = OpLabel
%27 = OpLoad %int %_1_loop
%29 = OpSLessThan %bool %27 %int_1
OpBranchConditional %29 %24 %26
%24 = OpLabel
%30 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%32 = OpLoad %v4float %30
%33 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%34 = OpLoad %v4float %33
%35 = OpFMul %v4float %32 %34
OpStore %_0_blend %35
OpBranch %25
%25 = OpLabel
%36 = OpLoad %int %_1_loop
%37 = OpIAdd %int %36 %int_1
OpStore %_1_loop %37
OpBranch %22
%26 = OpLabel
%38 = OpLoad %v4float %_0_blend
OpStore %sk_FragColor %38
OpReturn
OpFunctionEnd
