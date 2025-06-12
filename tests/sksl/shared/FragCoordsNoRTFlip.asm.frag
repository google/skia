               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor %sk_FragCoord
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %3
               OpName %sk_FragCoord "sk_FragCoord"  ; id %7
               OpName %main "main"                  ; id %2

               ; Annotations
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %sk_FragCoord BuiltIn FragCoord
               OpDecorate %15 RelaxedPrecision
               OpDecorate %16 RelaxedPrecision

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


               ; Function main
       %main = OpFunction %void None %10

         %11 = OpLabel
         %12 =   OpLoad %v4float %sk_FragCoord
         %13 =   OpVectorShuffle %v2float %12 %12 1 0
         %15 =   OpLoad %v4float %sk_FragColor      ; RelaxedPrecision
         %16 =   OpVectorShuffle %v4float %15 %13 4 5 2 3   ; RelaxedPrecision
                 OpStore %sk_FragColor %16
                 OpReturn
               OpFunctionEnd
