               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor %sk_FragCoord
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %sk_FragCoord "sk_FragCoord"  ; id %11
               OpName %main "main"                  ; id %6
               OpName %sksl_synthetic_uniforms "sksl_synthetic_uniforms"    ; id %24
               OpMemberName %sksl_synthetic_uniforms 0 "u_skRTFlip"

               ; Annotations
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %sk_FragCoord BuiltIn FragCoord
               OpDecorate %19 RelaxedPrecision
               OpDecorate %21 RelaxedPrecision
               OpMemberDecorate %sksl_synthetic_uniforms 0 Offset 16384
               OpDecorate %sksl_synthetic_uniforms Block
               OpDecorate %22 Binding 0
               OpDecorate %22 DescriptorSet 0

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
%_ptr_Output_float = OpTypePointer Output %float
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
    %v2float = OpTypeVector %float 2
%sksl_synthetic_uniforms = OpTypeStruct %v2float    ; Block
%_ptr_Uniform_sksl_synthetic_uniforms = OpTypePointer Uniform %sksl_synthetic_uniforms
         %22 = OpVariable %_ptr_Uniform_sksl_synthetic_uniforms Uniform     ; Binding 0, DescriptorSet 0
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float


               ; Function main
       %main = OpFunction %void None %14

         %15 = OpLabel
         %16 =   OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
         %19 =   OpLoad %float %16                  ; RelaxedPrecision
         %21 =   OpFSub %float %19 %float_1         ; RelaxedPrecision
                 OpStore %16 %21
         %26 =   OpAccessChain %_ptr_Uniform_v2float %22 %int_0
         %28 =   OpLoad %v2float %26
         %29 =   OpCompositeExtract %float %28 0
         %30 =   OpAccessChain %_ptr_Uniform_v2float %22 %int_0
         %31 =   OpLoad %v2float %30
         %32 =   OpCompositeExtract %float %31 1
         %33 =   OpLoad %v4float %sk_FragCoord
         %34 =   OpCompositeExtract %float %33 0
         %35 =   OpLoad %v4float %sk_FragCoord
         %36 =   OpCompositeExtract %float %35 1
         %37 =   OpLoad %v4float %sk_FragCoord
         %38 =   OpVectorShuffle %v2float %37 %37 2 3
         %39 =   OpFMul %float %32 %36
         %40 =   OpFAdd %float %29 %39
         %41 =   OpCompositeConstruct %v4float %34 %40 %38
         %42 =   OpCompositeConstruct %v4float %19 %19 %19 %19
         %43 =   OpFAdd %v4float %42 %41
                 OpReturn
               OpFunctionEnd
