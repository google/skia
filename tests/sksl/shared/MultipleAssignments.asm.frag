               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %3
               OpName %_entrypoint_v "_entrypoint_v"    ; id %7
               OpName %main "main"                      ; id %2
               OpName %x "x"                            ; id %20
               OpName %y "y"                            ; id %22
               OpName %a "a"                            ; id %24
               OpName %b "b"                            ; id %25
               OpName %c "c"                            ; id %26

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %a RelaxedPrecision
               OpDecorate %b RelaxedPrecision
               OpDecorate %c RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision

               ; Types, variables and constants
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
       %void = OpTypeVoid
          %9 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %13 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %17 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
    %float_1 = OpConstant %float 1


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %9

         %10 = OpLabel
         %14 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %14 %13
         %16 =   OpFunctionCall %v4float %main %14
                 OpStore %sk_FragColor %16
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %17         ; RelaxedPrecision
         %18 = OpFunctionParameter %_ptr_Function_v2float

         %19 = OpLabel
          %x =   OpVariable %_ptr_Function_float Function
          %y =   OpVariable %_ptr_Function_float Function
          %a =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
          %b =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
          %c =   OpVariable %_ptr_Function_float Function   ; RelaxedPrecision
                 OpStore %y %float_1
                 OpStore %x %float_1
                 OpStore %c %float_0
                 OpStore %b %float_0
                 OpStore %a %float_0
         %27 =   OpFMul %float %float_0 %float_0    ; RelaxedPrecision
         %28 =   OpCompositeConstruct %v4float %27 %float_1 %float_0 %float_1   ; RelaxedPrecision
                 OpReturnValue %28
               OpFunctionEnd
