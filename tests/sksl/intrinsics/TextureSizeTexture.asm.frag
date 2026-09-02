               OpCapability ImageQuery
               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %t "t"                        ; id %11
               OpName %main "main"                  ; id %6
               OpName %dims "dims"                  ; id %17

               ; Annotations
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %t Binding 0
               OpDecorate %t DescriptorSet 0
               OpDecorate %22 RelaxedPrecision
               OpDecorate %25 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
         %12 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_UniformConstant_12 = OpTypePointer UniformConstant %12
          %t = OpVariable %_ptr_UniformConstant_12 UniformConstant  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
     %v2uint = OpTypeVector %uint 2
%_ptr_Function_v2uint = OpTypePointer Function %v2uint
      %int_0 = OpConstant %int 0
    %v2float = OpTypeVector %float 2


               ; Function main
       %main = OpFunction %void None %15

         %16 = OpLabel
       %dims =   OpVariable %_ptr_Function_v2uint Function
         %22 =   OpLoad %12 %t                      ; RelaxedPrecision
         %21 =   OpImageQuerySizeLod %v2uint %22 %int_0
                 OpStore %dims %21
         %24 =   OpCompositeExtract %uint %21 0
         %25 =   OpConvertUToF %float %24           ; RelaxedPrecision
         %26 =   OpCompositeExtract %uint %21 1
         %27 =   OpConvertUToF %float %26           ; RelaxedPrecision
         %29 =   OpCompositeConstruct %v2float %25 %27  ; RelaxedPrecision
         %30 =   OpCompositeExtract %float %29 0        ; RelaxedPrecision
         %31 =   OpCompositeExtract %float %29 1        ; RelaxedPrecision
         %32 =   OpConvertUToF %float %24               ; RelaxedPrecision
         %33 =   OpConvertUToF %float %26               ; RelaxedPrecision
         %34 =   OpCompositeConstruct %v2float %32 %33  ; RelaxedPrecision
         %35 =   OpCompositeExtract %float %34 0        ; RelaxedPrecision
         %36 =   OpCompositeExtract %float %34 1        ; RelaxedPrecision
         %37 =   OpCompositeConstruct %v4float %30 %31 %35 %36  ; RelaxedPrecision
                 OpStore %sk_FragColor %37
                 OpReturn
               OpFunctionEnd
