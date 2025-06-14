               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor %sk_FragCoord
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %sk_FragCoord "sk_FragCoord"  ; id %11
               OpName %main "main"                  ; id %6

               ; Annotations
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %sk_FragCoord BuiltIn FragCoord
               OpDecorate %19 RelaxedPrecision
               OpDecorate %20 RelaxedPrecision

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


               ; Function main
       %main = OpFunction %void None %14

         %15 = OpLabel
         %16 =   OpLoad %v4float %sk_FragCoord
         %17 =   OpVectorShuffle %v2float %16 %16 1 0
         %19 =   OpLoad %v4float %sk_FragColor      ; RelaxedPrecision
         %20 =   OpVectorShuffle %v4float %19 %17 4 5 2 3   ; RelaxedPrecision
                 OpStore %sk_FragColor %20
                 OpReturn
               OpFunctionEnd
