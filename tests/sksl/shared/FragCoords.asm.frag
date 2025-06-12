               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor %sk_FragCoord
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %3
               OpName %sk_FragCoord "sk_FragCoord"  ; id %7
               OpName %main "main"                  ; id %2
               OpName %sksl_synthetic_uniforms "sksl_synthetic_uniforms"    ; id %14
               OpMemberName %sksl_synthetic_uniforms 0 "u_skRTFlip"

               ; Annotations
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %sk_FragCoord BuiltIn FragCoord
               OpMemberDecorate %sksl_synthetic_uniforms 0 Offset 16384
               OpDecorate %sksl_synthetic_uniforms Block
               OpDecorate %12 Binding 0
               OpDecorate %12 DescriptorSet 0
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision

               ; Types, variables and constants
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input    ; BuiltIn FragCoord
       %void = OpTypeVoid
         %10 = OpTypeFunction %void
    %v2float = OpTypeVector %float 2
%sksl_synthetic_uniforms = OpTypeStruct %v2float    ; Block
%_ptr_Uniform_sksl_synthetic_uniforms = OpTypePointer Uniform %sksl_synthetic_uniforms
         %12 = OpVariable %_ptr_Uniform_sksl_synthetic_uniforms Uniform     ; Binding 0, DescriptorSet 0
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float


               ; Function main
       %main = OpFunction %void None %10

         %11 = OpLabel
         %18 =   OpAccessChain %_ptr_Uniform_v2float %12 %int_0
         %20 =   OpLoad %v2float %18
         %21 =   OpCompositeExtract %float %20 0
         %22 =   OpAccessChain %_ptr_Uniform_v2float %12 %int_0
         %23 =   OpLoad %v2float %22
         %24 =   OpCompositeExtract %float %23 1
         %25 =   OpLoad %v4float %sk_FragCoord
         %26 =   OpCompositeExtract %float %25 0
         %27 =   OpLoad %v4float %sk_FragCoord
         %28 =   OpCompositeExtract %float %27 1
         %29 =   OpLoad %v4float %sk_FragCoord
         %30 =   OpVectorShuffle %v2float %29 %29 2 3
         %31 =   OpFMul %float %24 %28
         %32 =   OpFAdd %float %21 %31
         %33 =   OpCompositeConstruct %v4float %26 %32 %30
         %34 =   OpVectorShuffle %v2float %33 %33 1 0
         %35 =   OpLoad %v4float %sk_FragColor      ; RelaxedPrecision
         %36 =   OpVectorShuffle %v4float %35 %34 4 5 2 3   ; RelaxedPrecision
                 OpStore %sk_FragColor %36
                 OpReturn
               OpFunctionEnd
