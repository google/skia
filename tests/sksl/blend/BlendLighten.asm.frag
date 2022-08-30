OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
OpExecutionMode %main OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "src"
OpMemberName %_UniformBuffer 1 "dst"
OpName %blend_src_over_h4h4h4 "blend_src_over_h4h4h4"
OpName %blend_lighten_h4h4h4 "blend_lighten_h4h4h4"
OpName %result "result"
OpName %main "main"
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
OpDecorate %20 RelaxedPrecision
OpDecorate %22 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %result RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
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
%_ptr_Function_v4float = OpTypePointer Function %v4float
%16 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%float_1 = OpConstant %float 1
%v3float = OpTypeVector %float 3
%void = OpTypeVoid
%52 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%blend_src_over_h4h4h4 = OpFunction %v4float None %16
%17 = OpFunctionParameter %_ptr_Function_v4float
%18 = OpFunctionParameter %_ptr_Function_v4float
%19 = OpLabel
%20 = OpLoad %v4float %17
%22 = OpLoad %v4float %17
%23 = OpCompositeExtract %float %22 3
%24 = OpFSub %float %float_1 %23
%25 = OpLoad %v4float %18
%26 = OpVectorTimesScalar %v4float %25 %24
%27 = OpFAdd %v4float %20 %26
OpReturnValue %27
OpFunctionEnd
%blend_lighten_h4h4h4 = OpFunction %v4float None %16
%28 = OpFunctionParameter %_ptr_Function_v4float
%29 = OpFunctionParameter %_ptr_Function_v4float
%30 = OpLabel
%result = OpVariable %_ptr_Function_v4float Function
%33 = OpVariable %_ptr_Function_v4float Function
%35 = OpVariable %_ptr_Function_v4float Function
%32 = OpLoad %v4float %28
OpStore %33 %32
%34 = OpLoad %v4float %29
OpStore %35 %34
%36 = OpFunctionCall %v4float %blend_src_over_h4h4h4 %33 %35
OpStore %result %36
%38 = OpVectorShuffle %v3float %36 %36 0 1 2
%40 = OpLoad %v4float %29
%41 = OpCompositeExtract %float %40 3
%42 = OpFSub %float %float_1 %41
%43 = OpLoad %v4float %28
%44 = OpVectorShuffle %v3float %43 %43 0 1 2
%45 = OpVectorTimesScalar %v3float %44 %42
%46 = OpLoad %v4float %29
%47 = OpVectorShuffle %v3float %46 %46 0 1 2
%48 = OpFAdd %v3float %45 %47
%37 = OpExtInst %v3float %1 FMax %38 %48
%49 = OpLoad %v4float %result
%50 = OpVectorShuffle %v4float %49 %37 4 5 6 3
OpStore %result %50
OpReturnValue %50
OpFunctionEnd
%main = OpFunction %void None %52
%53 = OpLabel
%59 = OpVariable %_ptr_Function_v4float Function
%63 = OpVariable %_ptr_Function_v4float Function
%54 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%58 = OpLoad %v4float %54
OpStore %59 %58
%60 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%62 = OpLoad %v4float %60
OpStore %63 %62
%64 = OpFunctionCall %v4float %blend_lighten_h4h4h4 %59 %63
OpStore %sk_FragColor %64
OpReturn
OpFunctionEnd
