               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %3
               OpName %t "t"                        ; id %7
               OpName %_entrypoint_v "_entrypoint_v"    ; id %11
               OpName %main "main"                      ; id %2
               OpName %sksl_synthetic_uniforms "sksl_synthetic_uniforms"    ; id %32
               OpMemberName %sksl_synthetic_uniforms 0 "u_skRTFlip"

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %t RelaxedPrecision
               OpDecorate %t Binding 0
               OpDecorate %t DescriptorSet 0
               OpDecorate %24 RelaxedPrecision
               OpDecorate %25 RelaxedPrecision
               OpMemberDecorate %sksl_synthetic_uniforms 0 Offset 16384
               OpDecorate %sksl_synthetic_uniforms Block
               OpDecorate %31 Binding 0
               OpDecorate %31 DescriptorSet 0

               ; Types, variables and constants
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
          %8 = OpTypeImage %float 2D 0 0 0 1 Unknown
          %9 = OpTypeSampledImage %8
%_ptr_UniformConstant_9 = OpTypePointer UniformConstant %9
          %t = OpVariable %_ptr_UniformConstant_9 UniformConstant   ; RelaxedPrecision, Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %13 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %17 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %21 = OpTypeFunction %v4float %_ptr_Function_v2float
%sksl_synthetic_uniforms = OpTypeStruct %v2float    ; Block
%_ptr_Uniform_sksl_synthetic_uniforms = OpTypePointer Uniform %sksl_synthetic_uniforms
         %31 = OpVariable %_ptr_Uniform_sksl_synthetic_uniforms Uniform     ; Binding 0, DescriptorSet 0
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %13

         %14 = OpLabel
         %18 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %18 %17
         %20 =   OpFunctionCall %v4float %main %18
                 OpStore %sk_FragColor %20
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %21         ; RelaxedPrecision
         %22 = OpFunctionParameter %_ptr_Function_v2float

         %23 = OpLabel
         %25 =   OpLoad %9 %t                       ; RelaxedPrecision
         %26 =   OpLoad %v2float %22
         %28 =   OpLoad %v2float %22
         %27 =   OpDPdx %v2float %28
         %30 =   OpLoad %v2float %22
         %29 =   OpDPdy %v2float %30
         %36 =   OpAccessChain %_ptr_Uniform_v2float %31 %int_0
         %38 =   OpLoad %v2float %36
         %39 =   OpVectorShuffle %v2float %38 %38 1 1
         %40 =   OpFMul %v2float %29 %39
         %24 =   OpImageSampleExplicitLod %v4float %25 %26 Grad %27 %40     ; RelaxedPrecision
                 OpReturnValue %24
               OpFunctionEnd
