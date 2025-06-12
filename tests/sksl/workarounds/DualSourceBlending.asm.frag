               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor %sk_SecondaryFragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %3
               OpName %sk_SecondaryFragColor "sk_SecondaryFragColor"    ; id %7
               OpName %main "main"                                      ; id %2

               ; Annotations
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %sk_SecondaryFragColor RelaxedPrecision
               OpDecorate %sk_SecondaryFragColor Location 0
               OpDecorate %sk_SecondaryFragColor Index 1
               OpDecorate %11 RelaxedPrecision

               ; Types, variables and constants
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%sk_SecondaryFragColor = OpVariable %_ptr_Output_v4float Output     ; RelaxedPrecision, Location 0, Index 1
       %void = OpTypeVoid
          %9 = OpTypeFunction %void


               ; Function main
       %main = OpFunction %void None %9

         %10 = OpLabel
         %11 =   OpLoad %v4float %sk_SecondaryFragColor     ; RelaxedPrecision
                 OpStore %sk_FragColor %11
                 OpReturn
               OpFunctionEnd
