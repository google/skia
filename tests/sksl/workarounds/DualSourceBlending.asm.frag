               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor %sk_SecondaryFragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %sk_SecondaryFragColor "sk_SecondaryFragColor"    ; id %11
               OpName %main "main"                                      ; id %6

               ; Annotations
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %sk_SecondaryFragColor RelaxedPrecision
               OpDecorate %sk_SecondaryFragColor Location 0
               OpDecorate %sk_SecondaryFragColor Index 1
               OpDecorate %15 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%sk_SecondaryFragColor = OpVariable %_ptr_Output_v4float Output     ; RelaxedPrecision, Location 0, Index 1
       %void = OpTypeVoid
         %13 = OpTypeFunction %void


               ; Function main
       %main = OpFunction %void None %13

         %14 = OpLabel
         %15 =   OpLoad %v4float %sk_SecondaryFragColor     ; RelaxedPrecision
                 OpStore %sk_FragColor %15
                 OpReturn
               OpFunctionEnd
