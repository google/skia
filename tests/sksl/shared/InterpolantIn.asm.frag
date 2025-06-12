               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor %defaultVarying %linearVarying %flatVarying
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %3
               OpName %defaultVarying "defaultVarying"  ; id %7
               OpName %linearVarying "linearVarying"    ; id %9
               OpName %flatVarying "flatVarying"        ; id %10
               OpName %main "main"                      ; id %2

               ; Annotations
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %defaultVarying Location 0
               OpDecorate %linearVarying Location 1
               OpDecorate %linearVarying NoPerspective
               OpDecorate %flatVarying Location 2
               OpDecorate %flatVarying Flat
               OpDecorate %18 RelaxedPrecision

               ; Types, variables and constants
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_ptr_Input_float = OpTypePointer Input %float
%defaultVarying = OpVariable %_ptr_Input_float Input    ; Location 0
%linearVarying = OpVariable %_ptr_Input_float Input     ; Location 1, NoPerspective
%flatVarying = OpVariable %_ptr_Input_float Input       ; Location 2, Flat
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_1 = OpConstant %float 1


               ; Function main
       %main = OpFunction %void None %12

         %13 = OpLabel
         %14 =   OpLoad %float %defaultVarying
         %15 =   OpLoad %float %linearVarying
         %16 =   OpLoad %float %flatVarying
         %18 =   OpCompositeConstruct %v4float %14 %15 %16 %float_1     ; RelaxedPrecision
                 OpStore %sk_FragColor %18
                 OpReturn
               OpFunctionEnd
