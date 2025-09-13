               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %main "main"                  ; id %6
               OpName %i "i"                        ; id %14

               ; Annotations
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %19 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
%_ptr_Function_int = OpTypePointer Function %int
      %int_1 = OpConstant %int 1
      %int_3 = OpConstant %int 3
%_ptr_Output_float = OpTypePointer Output %float
      %int_0 = OpConstant %int 0


               ; Function main
       %main = OpFunction %void None %12

         %13 = OpLabel
          %i =   OpVariable %_ptr_Function_int Function
                 OpStore %i %int_1
                 OpStore %i %int_3
         %18 =   OpIMul %int %int_1 %int_3
         %19 =   OpConvertSToF %float %int_3        ; RelaxedPrecision
         %20 =   OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
                 OpStore %20 %19
                 OpReturn
               OpFunctionEnd
