               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor %sk_FragCoord
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %sk_FragCoord "sk_FragCoord"  ; id %11
               OpName %main "main"                  ; id %6
               OpName %sksl_synthetic_uniforms "sksl_synthetic_uniforms"    ; id %18
               OpMemberName %sksl_synthetic_uniforms 0 "u_skRTFlip"

               ; Annotations
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %sk_FragCoord BuiltIn FragCoord
               OpMemberDecorate %sksl_synthetic_uniforms 0 Offset 16384
               OpDecorate %sksl_synthetic_uniforms Block
               OpDecorate %16 Binding 0
               OpDecorate %16 DescriptorSet 0
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_ptr_Input_v4float = OpTypePointer Input %v4float
%sk_FragCoord = OpVariable %_ptr_Input_v4float Input    ; BuiltIn FragCoord
       %void = OpTypeVoid
         %14 = OpTypeFunction %void
    %v2float = OpTypeVector %float 2
%sksl_synthetic_uniforms = OpTypeStruct %v2float    ; Block
%_ptr_Uniform_sksl_synthetic_uniforms = OpTypePointer Uniform %sksl_synthetic_uniforms
         %16 = OpVariable %_ptr_Uniform_sksl_synthetic_uniforms Uniform     ; Binding 0, DescriptorSet 0
      %int_0 = OpConstant %int 0
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float


               ; Function main
       %main = OpFunction %void None %14

         %15 = OpLabel
         %21 =   OpAccessChain %_ptr_Uniform_v2float %16 %int_0
         %23 =   OpLoad %v2float %21
         %24 =   OpCompositeExtract %float %23 0
         %25 =   OpAccessChain %_ptr_Uniform_v2float %16 %int_0
         %26 =   OpLoad %v2float %25
         %27 =   OpCompositeExtract %float %26 1
         %28 =   OpLoad %v4float %sk_FragCoord
         %29 =   OpCompositeExtract %float %28 0
         %30 =   OpLoad %v4float %sk_FragCoord
         %31 =   OpCompositeExtract %float %30 1
         %32 =   OpLoad %v4float %sk_FragCoord
         %33 =   OpVectorShuffle %v2float %32 %32 2 3
         %34 =   OpFMul %float %27 %31
         %35 =   OpFAdd %float %24 %34
         %36 =   OpCompositeConstruct %v4float %29 %35 %33
         %37 =   OpVectorShuffle %v2float %36 %36 1 0
         %38 =   OpLoad %v4float %sk_FragColor      ; RelaxedPrecision
         %39 =   OpVectorShuffle %v4float %38 %37 4 5 2 3   ; RelaxedPrecision
                 OpStore %sk_FragColor %39
                 OpReturn
               OpFunctionEnd
