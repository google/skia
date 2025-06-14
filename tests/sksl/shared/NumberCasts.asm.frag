               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_entrypoint_v "_entrypoint_v"    ; id %11
               OpName %main "main"                      ; id %6
               OpName %B "B"                            ; id %24
               OpName %F "F"                            ; id %36
               OpName %I "I"                            ; id %45

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %61 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision

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
       %bool = OpTypeBool
     %v3bool = OpTypeVector %bool 3
%_ptr_Function_v3bool = OpTypePointer Function %v3bool
       %true = OpConstantTrue %bool
%_ptr_Function_bool = OpTypePointer Function %bool
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
    %v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%float_1_23000002 = OpConstant %float 1.23000002
%_ptr_Function_float = OpTypePointer Function %float
    %float_1 = OpConstant %float 1
      %v3int = OpTypeVector %int 3
%_ptr_Function_v3int = OpTypePointer Function %v3int
%_ptr_Function_int = OpTypePointer Function %int
      %false = OpConstantFalse %bool


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
          %B =   OpVariable %_ptr_Function_v3bool Function
          %F =   OpVariable %_ptr_Function_v3float Function
          %I =   OpVariable %_ptr_Function_v3int Function
         %29 =   OpAccessChain %_ptr_Function_bool %B %int_0
                 OpStore %29 %true
         %32 =   OpAccessChain %_ptr_Function_bool %B %int_1
                 OpStore %32 %true
         %34 =   OpAccessChain %_ptr_Function_bool %B %int_2
                 OpStore %34 %true
         %40 =   OpAccessChain %_ptr_Function_float %F %int_0
                 OpStore %40 %float_1_23000002
         %42 =   OpAccessChain %_ptr_Function_float %F %int_1
                 OpStore %42 %float_0
         %44 =   OpAccessChain %_ptr_Function_float %F %int_2
                 OpStore %44 %float_1
         %48 =   OpAccessChain %_ptr_Function_int %I %int_0
                 OpStore %48 %int_1
         %50 =   OpAccessChain %_ptr_Function_int %I %int_1
                 OpStore %50 %int_1
         %51 =   OpAccessChain %_ptr_Function_int %I %int_2
                 OpStore %51 %int_1
         %52 =   OpLoad %v3float %F
         %53 =   OpCompositeExtract %float %52 0
         %54 =   OpLoad %v3float %F
         %55 =   OpCompositeExtract %float %54 1
         %56 =   OpFMul %float %53 %55
         %57 =   OpLoad %v3float %F
         %58 =   OpCompositeExtract %float %57 2
         %59 =   OpFMul %float %56 %58
         %61 =   OpLoad %v3bool %B                  ; RelaxedPrecision
         %62 =   OpCompositeExtract %bool %61 0
                 OpSelectionMerge %64 None
                 OpBranchConditional %62 %63 %64

         %63 =     OpLabel
         %65 =       OpLoad %v3bool %B              ; RelaxedPrecision
         %66 =       OpCompositeExtract %bool %65 1
                     OpBranch %64

         %64 = OpLabel
         %67 =   OpPhi %bool %false %23 %66 %63
                 OpSelectionMerge %69 None
                 OpBranchConditional %67 %68 %69

         %68 =     OpLabel
         %70 =       OpLoad %v3bool %B              ; RelaxedPrecision
         %71 =       OpCompositeExtract %bool %70 2
                     OpBranch %69

         %69 = OpLabel
         %72 =   OpPhi %bool %false %64 %71 %68
         %73 =   OpSelect %float %72 %float_1 %float_0  ; RelaxedPrecision
         %74 =   OpLoad %v3int %I
         %75 =   OpCompositeExtract %int %74 0
         %76 =   OpLoad %v3int %I
         %77 =   OpCompositeExtract %int %76 1
         %78 =   OpIMul %int %75 %77
         %79 =   OpLoad %v3int %I
         %80 =   OpCompositeExtract %int %79 2
         %81 =   OpIMul %int %78 %80
         %82 =   OpConvertSToF %float %81           ; RelaxedPrecision
         %83 =   OpCompositeConstruct %v4float %59 %73 %float_0 %82     ; RelaxedPrecision
                 OpReturnValue %83
               OpFunctionEnd
