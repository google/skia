OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %test2D "test2D"
OpName %test2DRect "test2DRect"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %test2D RelaxedPrecision
OpDecorate %test2D Binding 0
OpDecorate %test2D DescriptorSet 0
OpDecorate %test2DRect RelaxedPrecision
OpDecorate %test2DRect Binding 1
OpDecorate %test2DRect DescriptorSet 0
OpDecorate %19 RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%13 = OpTypeImage %float 2D 0 0 0 1 Unknown
%12 = OpTypeSampledImage %13
%_ptr_UniformConstant_12 = OpTypePointer UniformConstant %12
%test2D = OpVariable %_ptr_UniformConstant_12 UniformConstant
%test2DRect = OpVariable %_ptr_UniformConstant_12 UniformConstant
%void = OpTypeVoid
%16 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0_5 = OpConstant %float 0.5
%22 = OpConstantComposite %v2float %float_0_5 %float_0_5
%v3float = OpTypeVector %float 3
%28 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
%main = OpFunction %void None %16
%17 = OpLabel
%19 = OpLoad %12 %test2D
%18 = OpImageSampleImplicitLod %v4float %19 %22
OpStore %sk_FragColor %18
%24 = OpLoad %12 %test2DRect
%23 = OpImageSampleImplicitLod %v4float %24 %22
OpStore %sk_FragColor %23
%26 = OpLoad %12 %test2DRect
%25 = OpImageSampleProjImplicitLod %v4float %26 %28
OpStore %sk_FragColor %25
OpReturn
OpFunctionEnd
