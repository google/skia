               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor %defaultVarying %linearVarying %flatVarying
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %defaultVarying "defaultVarying"  ; id %11
               OpName %linearVarying "linearVarying"    ; id %13
               OpName %flatVarying "flatVarying"        ; id %14
               OpName %main "main"                      ; id %6

               ; Annotations
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %defaultVarying Location 0
               OpDecorate %linearVarying Location 1
               OpDecorate %linearVarying NoPerspective
               OpDecorate %flatVarying Location 2
               OpDecorate %flatVarying Flat
               OpDecorate %22 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_ptr_Input_float = OpTypePointer Input %float
%defaultVarying = OpVariable %_ptr_Input_float Input    ; Location 0
%linearVarying = OpVariable %_ptr_Input_float Input     ; Location 1, NoPerspective
%flatVarying = OpVariable %_ptr_Input_float Input       ; Location 2, Flat
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
    %float_1 = OpConstant %float 1


               ; Function main
       %main = OpFunction %void None %16

         %17 = OpLabel
         %18 =   OpLoad %float %defaultVarying
         %19 =   OpLoad %float %linearVarying
         %20 =   OpLoad %float %flatVarying
         %22 =   OpCompositeConstruct %v4float %18 %19 %20 %float_1     ; RelaxedPrecision
                 OpStore %sk_FragColor %22
                 OpReturn
               OpFunctionEnd
