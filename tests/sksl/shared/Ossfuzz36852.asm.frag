               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_entrypoint_v "_entrypoint_v"    ; id %11
               OpName %main "main"                      ; id %6
               OpName %x "x"                            ; id %24
               OpName %y "y"                            ; id %33

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %x RelaxedPrecision
               OpDecorate %35 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
       %void = OpTypeVoid
         %13 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %17 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %21 = OpTypeFunction %v4float %_ptr_Function_v2float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
         %30 = OpConstantComposite %v2float %float_0 %float_1
         %31 = OpConstantComposite %v2float %float_2 %float_3
         %32 = OpConstantComposite %mat2v2float %30 %31
         %34 = OpConstantComposite %v4float %float_0 %float_1 %float_2 %float_3


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %13

         %14 = OpLabel
         %18 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %18 %17
         %20 =   OpFunctionCall %v4float %main %18
                 OpStore %sk_FragColor %20
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %21         ; RelaxedPrecision
         %22 = OpFunctionParameter %_ptr_Function_v2float

         %23 = OpLabel
          %x =   OpVariable %_ptr_Function_mat2v2float Function     ; RelaxedPrecision
          %y =   OpVariable %_ptr_Function_v2float Function
                 OpStore %x %32
         %35 =   OpVectorShuffle %v2float %34 %34 0 1   ; RelaxedPrecision
                 OpStore %y %35
         %36 =   OpCompositeExtract %float %35 0
         %37 =   OpCompositeExtract %float %35 1
         %38 =   OpCompositeConstruct %v4float %36 %37 %float_0 %float_1
                 OpReturnValue %38
               OpFunctionEnd
