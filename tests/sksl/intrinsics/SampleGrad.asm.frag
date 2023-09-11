               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %t "t"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %sksl_synthetic_uniforms "sksl_synthetic_uniforms"
               OpMemberName %sksl_synthetic_uniforms 0 "u_skRTFlip"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %t RelaxedPrecision
               OpDecorate %t Binding 0
               OpDecorate %t DescriptorSet 0
               OpDecorate %28 RelaxedPrecision
               OpMemberDecorate %sksl_synthetic_uniforms 0 Offset 16384
               OpDecorate %sksl_synthetic_uniforms Block
               OpDecorate %34 Binding 0
               OpDecorate %34 DescriptorSet 0
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
         %11 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %12 = OpTypeSampledImage %11
%_ptr_UniformConstant_12 = OpTypePointer UniformConstant %12
          %t = OpVariable %_ptr_UniformConstant_12 UniformConstant
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %24 = OpTypeFunction %v4float %_ptr_Function_v2float
%sksl_synthetic_uniforms = OpTypeStruct %v2float
%_ptr_Uniform_sksl_synthetic_uniforms = OpTypePointer Uniform %sksl_synthetic_uniforms
         %34 = OpVariable %_ptr_Uniform_sksl_synthetic_uniforms Uniform
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%_entrypoint_v = OpFunction %void None %16
         %17 = OpLabel
         %21 = OpVariable %_ptr_Function_v2float Function
               OpStore %21 %20
         %23 = OpFunctionCall %v4float %main %21
               OpStore %sk_FragColor %23
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %24
         %25 = OpFunctionParameter %_ptr_Function_v2float
         %26 = OpLabel
         %28 = OpLoad %12 %t
         %29 = OpLoad %v2float %25
         %31 = OpLoad %v2float %25
         %30 = OpDPdx %v2float %31
         %33 = OpLoad %v2float %25
         %32 = OpDPdy %v2float %33
         %39 = OpAccessChain %_ptr_Uniform_v2float %34 %int_0
         %41 = OpLoad %v2float %39
         %42 = OpVectorShuffle %v2float %41 %41 1 1
         %43 = OpFMul %v2float %32 %42
         %27 = OpImageSampleExplicitLod %v4float %28 %29 Grad %30 %43
               OpReturnValue %27
               OpFunctionEnd
