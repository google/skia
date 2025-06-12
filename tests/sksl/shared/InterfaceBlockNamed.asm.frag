               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %testBlock "testBlock"        ; id %5
               OpMemberName %testBlock 0 "x"
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %main "main"                  ; id %2

               ; Annotations
               OpMemberDecorate %testBlock 0 Offset 0
               OpDecorate %testBlock Block
               OpDecorate %3 Binding 456
               OpDecorate %3 DescriptorSet 0
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %18 RelaxedPrecision

               ; Types, variables and constants
      %float = OpTypeFloat 32
  %testBlock = OpTypeStruct %float                  ; Block
%_ptr_Uniform_testBlock = OpTypePointer Uniform %testBlock
          %3 = OpVariable %_ptr_Uniform_testBlock Uniform   ; Binding 456, DescriptorSet 0
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
       %void = OpTypeVoid
         %11 = OpTypeFunction %void
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Uniform_float = OpTypePointer Uniform %float


               ; Function main
       %main = OpFunction %void None %11

         %12 = OpLabel
         %15 =   OpAccessChain %_ptr_Uniform_float %3 %int_0
         %17 =   OpLoad %float %15
         %18 =   OpCompositeConstruct %v4float %17 %17 %17 %17  ; RelaxedPrecision
                 OpStore %sk_FragColor %18
                 OpReturn
               OpFunctionEnd
