               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %t "t"                        ; id %11
               OpName %main "main"                  ; id %6
               OpName %c "c"                        ; id %18

               ; Annotations
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %t RelaxedPrecision
               OpDecorate %t Binding 0
               OpDecorate %t DescriptorSet 0
               OpDecorate %c RelaxedPrecision
               OpDecorate %20 RelaxedPrecision
               OpDecorate %21 RelaxedPrecision
               OpDecorate %25 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
         %12 = OpTypeImage %float 2D 0 0 0 1 Unknown
         %13 = OpTypeSampledImage %12
%_ptr_UniformConstant_13 = OpTypePointer UniformConstant %13
          %t = OpVariable %_ptr_UniformConstant_13 UniformConstant  ; RelaxedPrecision, Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %24 = OpConstantComposite %v2float %float_0 %float_0
    %float_1 = OpConstant %float 1
    %v3float = OpTypeVector %float 3
         %29 = OpConstantComposite %v3float %float_1 %float_1 %float_1


               ; Function main
       %main = OpFunction %void None %16

         %17 = OpLabel
          %c =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %21 =   OpLoad %13 %t                                  ; RelaxedPrecision
         %20 =   OpImageSampleImplicitLod %v4float %21 %24      ; RelaxedPrecision
                 OpStore %c %20
         %26 =   OpLoad %13 %t                      ; RelaxedPrecision
         %25 =   OpImageSampleProjImplicitLod %v4float %26 %29  ; RelaxedPrecision
         %30 =   OpFMul %v4float %20 %25                        ; RelaxedPrecision
                 OpStore %sk_FragColor %30
                 OpReturn
               OpFunctionEnd
