               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %zero "zero"                  ; id %11
               OpName %globalArray "globalArray"    ; id %14
               OpName %globalMatrix "globalMatrix"  ; id %20
               OpName %_entrypoint_v "_entrypoint_v"    ; id %26
               OpName %main "main"                      ; id %6
               OpName %localArray "localArray"          ; id %38
               OpName %localMatrix "localMatrix"        ; id %41

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %globalArray RelaxedPrecision
               OpDecorate %_arr_float_int_2 ArrayStride 16
               OpDecorate %19 RelaxedPrecision
               OpDecorate %globalMatrix RelaxedPrecision
               OpDecorate %localArray RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %localMatrix RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_ptr_Private_int = OpTypePointer Private %int
       %zero = OpVariable %_ptr_Private_int Private
      %int_0 = OpConstant %int 0
      %int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2       ; ArrayStride 16
%_ptr_Private__arr_float_int_2 = OpTypePointer Private %_arr_float_int_2
%globalArray = OpVariable %_ptr_Private__arr_float_int_2 Private    ; RelaxedPrecision
    %float_1 = OpConstant %float 1
    %v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Private_mat2v2float = OpTypePointer Private %mat2v2float
%globalMatrix = OpVariable %_ptr_Private_mat2v2float Private    ; RelaxedPrecision
         %24 = OpConstantComposite %v2float %float_1 %float_1
         %25 = OpConstantComposite %mat2v2float %24 %24
       %void = OpTypeVoid
         %28 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %31 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %35 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function__arr_float_int_2 = OpTypePointer Function %_arr_float_int_2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
         %45 = OpConstantComposite %v2float %float_0 %float_1
         %46 = OpConstantComposite %v2float %float_2 %float_3
         %47 = OpConstantComposite %mat2v2float %45 %46
%_ptr_Private_float = OpTypePointer Private %float
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Private_v2float = OpTypePointer Private %v2float


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %28

         %29 = OpLabel
         %32 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %32 %31
         %34 =   OpFunctionCall %v4float %main %32
                 OpStore %sk_FragColor %34
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %35         ; RelaxedPrecision
         %36 = OpFunctionParameter %_ptr_Function_v2float

         %37 = OpLabel
 %localArray =   OpVariable %_ptr_Function__arr_float_int_2 Function    ; RelaxedPrecision
%localMatrix =   OpVariable %_ptr_Function_mat2v2float Function         ; RelaxedPrecision
                 OpStore %zero %int_0
         %19 =   OpCompositeConstruct %_arr_float_int_2 %float_1 %float_1   ; RelaxedPrecision
                 OpStore %globalArray %19
                 OpStore %globalMatrix %25
         %40 =   OpCompositeConstruct %_arr_float_int_2 %float_0 %float_1   ; RelaxedPrecision
                 OpStore %localArray %40
                 OpStore %localMatrix %47
         %48 =   OpLoad %int %zero
         %49 =   OpAccessChain %_ptr_Private_float %globalArray %48
         %51 =   OpLoad %float %49                  ; RelaxedPrecision
         %52 =   OpLoad %int %zero
         %53 =   OpAccessChain %_ptr_Function_float %localArray %52
         %55 =   OpLoad %float %53                  ; RelaxedPrecision
         %56 =   OpFMul %float %51 %55              ; RelaxedPrecision
         %57 =   OpLoad %int %zero
         %58 =   OpVectorExtractDynamic %float %24 %57
         %59 =   OpLoad %int %zero
         %60 =   OpVectorExtractDynamic %float %24 %59
         %61 =   OpFMul %float %58 %60              ; RelaxedPrecision
         %62 =   OpLoad %int %zero
         %63 =   OpAccessChain %_ptr_Private_v2float %globalMatrix %62
         %65 =   OpLoad %v2float %63                ; RelaxedPrecision
         %66 =   OpLoad %int %zero
         %67 =   OpAccessChain %_ptr_Function_v2float %localMatrix %66
         %68 =   OpLoad %v2float %67                ; RelaxedPrecision
         %69 =   OpFMul %v2float %65 %68            ; RelaxedPrecision
         %70 =   OpCompositeExtract %float %69 0    ; RelaxedPrecision
         %71 =   OpCompositeExtract %float %69 1    ; RelaxedPrecision
         %72 =   OpCompositeConstruct %v4float %56 %61 %70 %71  ; RelaxedPrecision
                 OpReturnValue %72
               OpFunctionEnd
