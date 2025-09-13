               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %8
               OpName %_entrypoint_v "_entrypoint_v"    ; id %12
               OpName %userfunc_ff "userfunc_ff"        ; id %6
               OpName %main "main"                      ; id %7
               OpName %b "b"                            ; id %32
               OpName %c "c"                            ; id %34
               OpName %x "x"                            ; id %47
               OpName %d "d"                            ; id %61

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %65 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
       %void = OpTypeVoid
         %14 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %18 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_float = OpTypePointer Function %float
         %23 = OpTypeFunction %float %_ptr_Function_float
    %float_1 = OpConstant %float 1
         %29 = OpTypeFunction %v4float %_ptr_Function_v2float
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
   %float_77 = OpConstant %float 77
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
    %float_5 = OpConstant %float 5
    %float_4 = OpConstant %float 4


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %14

         %15 = OpLabel
         %19 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %19 %18
         %21 =   OpFunctionCall %v4float %main %19
                 OpStore %sk_FragColor %21
                 OpReturn
               OpFunctionEnd


               ; Function userfunc_ff
%userfunc_ff = OpFunction %float None %23
         %24 = OpFunctionParameter %_ptr_Function_float

         %25 = OpLabel
         %26 =   OpLoad %float %24
         %28 =   OpFAdd %float %26 %float_1
                 OpReturnValue %28
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %29         ; RelaxedPrecision
         %30 = OpFunctionParameter %_ptr_Function_v2float

         %31 = OpLabel
          %b =   OpVariable %_ptr_Function_float Function
          %c =   OpVariable %_ptr_Function_float Function
         %41 =   OpVariable %_ptr_Function_float Function
         %44 =   OpVariable %_ptr_Function_float Function
          %x =   OpVariable %_ptr_Function_int Function
          %d =   OpVariable %_ptr_Function_float Function
                 OpStore %b %float_2
                 OpStore %c %float_3
                 OpStore %b %float_2
         %37 =   OpFAdd %float %float_3 %float_77
                 OpStore %b %37
         %39 =   OpFAdd %float %float_3 %float_77
         %38 =   OpExtInst %float %5 Sin %39
                 OpStore %b %38
         %40 =   OpFAdd %float %float_3 %float_77
                 OpStore %41 %40
         %42 =   OpFunctionCall %float %userfunc_ff %41
         %43 =   OpFAdd %float %float_3 %float_77
                 OpStore %44 %43
         %45 =   OpFunctionCall %float %userfunc_ff %44
                 OpStore %b %45
         %46 =   OpExtInst %float %5 Cos %float_3
                 OpStore %b %46
                 OpStore %b %46
                 OpStore %x %int_0
                 OpBranch %50

         %50 = OpLabel
                 OpLoopMerge %54 %53 None
                 OpBranch %51

         %51 =     OpLabel
         %55 =       OpLoad %int %x
         %57 =       OpSLessThan %bool %55 %int_1
                     OpBranchConditional %57 %52 %54

         %52 =         OpLabel
                         OpBranch %53

         %53 =   OpLabel
         %59 =     OpLoad %int %x
         %60 =     OpIAdd %int %59 %int_1
                   OpStore %x %60
                   OpBranch %50

         %54 = OpLabel
         %62 =   OpLoad %float %c
                 OpStore %d %62
                 OpStore %b %float_3
         %63 =   OpFAdd %float %62 %float_1
                 OpStore %d %63
         %64 =   OpFOrdEqual %bool %float_3 %float_2
         %65 =   OpSelect %float %64 %float_1 %float_0  ; RelaxedPrecision
         %67 =   OpSelect %float %true %float_1 %float_0    ; RelaxedPrecision
         %69 =   OpFOrdEqual %bool %63 %float_5
         %70 =   OpSelect %float %69 %float_1 %float_0  ; RelaxedPrecision
         %72 =   OpFOrdEqual %bool %63 %float_4
         %73 =   OpSelect %float %72 %float_1 %float_0  ; RelaxedPrecision
         %74 =   OpCompositeConstruct %v4float %65 %67 %70 %73  ; RelaxedPrecision
                 OpReturnValue %74
               OpFunctionEnd
