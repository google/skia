               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %testBlock "testBlock"        ; id %9
               OpMemberName %testBlock 0 "x"
               OpName %sk_FragColor "sk_FragColor"  ; id %11
               OpName %main "main"                  ; id %6

               ; Annotations
               OpMemberDecorate %testBlock 0 Offset 0
               OpDecorate %testBlock Block
               OpDecorate %7 Binding 456
               OpDecorate %7 DescriptorSet 0
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %21 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
  %testBlock = OpTypeStruct %float                  ; Block
%_ptr_Uniform_testBlock = OpTypePointer Uniform %testBlock
          %7 = OpVariable %_ptr_Uniform_testBlock Uniform   ; Binding 456, DescriptorSet 0
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
      %int_0 = OpConstant %int 0
%_ptr_Uniform_float = OpTypePointer Uniform %float


               ; Function main
       %main = OpFunction %void None %15

         %16 = OpLabel
         %18 =   OpAccessChain %_ptr_Uniform_float %7 %int_0
         %20 =   OpLoad %float %18
         %21 =   OpCompositeConstruct %v4float %20 %20 %20 %20  ; RelaxedPrecision
                 OpStore %sk_FragColor %21
                 OpReturn
               OpFunctionEnd
