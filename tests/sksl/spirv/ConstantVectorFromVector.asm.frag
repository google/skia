               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %3
               OpName %main "main"                  ; id %2

               ; Annotations
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0

               ; Types, variables and constants
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
       %void = OpTypeVoid
          %8 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
%_ptr_Output_float = OpTypePointer Output %float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0


               ; Function main
       %main = OpFunction %void None %8

          %9 = OpLabel
         %11 =   OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
                 OpStore %11 %float_0
                 OpReturn
               OpFunctionEnd
