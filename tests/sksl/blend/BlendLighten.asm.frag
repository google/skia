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
OpName %blend_lighten "blend_lighten"
OpName %result "result"
OpName %main "main"
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
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %20 RelaxedPrecision
OpDecorate %22 RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
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
%_ptr_Function_v4float = OpTypePointer Function %v4float
%14 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%float_1 = OpConstant %float 1
%v3float = OpTypeVector %float 3
%void = OpTypeVoid
%45 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%blend_lighten = OpFunction %v4float None %14
%16 = OpFunctionParameter %_ptr_Function_v4float
%17 = OpFunctionParameter %_ptr_Function_v4float
%18 = OpLabel
%result = OpVariable %_ptr_Function_v4float Function
%20 = OpLoad %v4float %16
%22 = OpLoad %v4float %16
%23 = OpCompositeExtract %float %22 3
%24 = OpFSub %float %float_1 %23
%25 = OpLoad %v4float %17
%26 = OpVectorTimesScalar %v4float %25 %24
%27 = OpFAdd %v4float %20 %26
OpStore %result %27
%29 = OpLoad %v4float %result
%30 = OpVectorShuffle %v3float %29 %29 0 1 2
%32 = OpLoad %v4float %17
%33 = OpCompositeExtract %float %32 3
%34 = OpFSub %float %float_1 %33
%35 = OpLoad %v4float %16
%36 = OpVectorShuffle %v3float %35 %35 0 1 2
%37 = OpVectorTimesScalar %v3float %36 %34
%38 = OpLoad %v4float %17
%39 = OpVectorShuffle %v3float %38 %38 0 1 2
%40 = OpFAdd %v3float %37 %39
%28 = OpExtInst %v3float %1 FMax %30 %40
%41 = OpLoad %v4float %result
%42 = OpVectorShuffle %v4float %41 %28 4 5 6 3
OpStore %result %42
%43 = OpLoad %v4float %result
OpReturnValue %43
OpFunctionEnd
%main = OpFunction %void None %45
%46 = OpLabel
%52 = OpVariable %_ptr_Function_v4float Function
%56 = OpVariable %_ptr_Function_v4float Function
%47 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%51 = OpLoad %v4float %47
OpStore %52 %51
%53 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%55 = OpLoad %v4float %53
OpStore %56 %55
%57 = OpFunctionCall %v4float %blend_lighten %52 %56
OpStore %sk_FragColor %57
OpReturn
OpFunctionEnd
