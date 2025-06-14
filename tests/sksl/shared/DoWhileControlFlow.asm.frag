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

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %x RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision

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
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_1 = OpConstant %float 1
         %27 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Function_float = OpTypePointer Function %float
      %int_0 = OpConstant %int 0
 %float_0_25 = OpConstant %float 0.25
       %bool = OpTypeBool
      %int_2 = OpConstant %int 2
      %int_1 = OpConstant %int 1


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
          %x =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
                 OpStore %x %27
                 OpBranch %28

         %28 = OpLabel
                 OpLoopMerge %32 %31 None
                 OpBranch %29

         %29 =     OpLabel
         %33 =       OpAccessChain %_ptr_Function_float %x %int_0
         %36 =       OpLoad %float %33              ; RelaxedPrecision
         %38 =       OpFSub %float %36 %float_0_25  ; RelaxedPrecision
                     OpStore %33 %38
         %39 =       OpLoad %v4float %x             ; RelaxedPrecision
         %40 =       OpCompositeExtract %float %39 0    ; RelaxedPrecision
         %41 =       OpFOrdLessThanEqual %bool %40 %float_0
                     OpSelectionMerge %44 None
                     OpBranchConditional %41 %43 %44

         %43 =         OpLabel
                         OpBranch %32

         %44 =     OpLabel
                     OpBranch %30

         %30 =     OpLabel
                     OpBranch %31

         %31 =   OpLabel
         %45 =     OpLoad %v4float %x               ; RelaxedPrecision
         %46 =     OpCompositeExtract %float %45 3  ; RelaxedPrecision
         %47 =     OpFOrdEqual %bool %46 %float_1
                   OpBranchConditional %47 %28 %32

         %32 = OpLabel
                 OpBranch %48

         %48 = OpLabel
                 OpLoopMerge %52 %51 None
                 OpBranch %49

         %49 =     OpLabel
         %53 =       OpAccessChain %_ptr_Function_float %x %int_2
         %55 =       OpLoad %float %53              ; RelaxedPrecision
         %56 =       OpFSub %float %55 %float_0_25  ; RelaxedPrecision
                     OpStore %53 %56
         %57 =       OpLoad %v4float %x             ; RelaxedPrecision
         %58 =       OpCompositeExtract %float %57 3    ; RelaxedPrecision
         %59 =       OpFOrdEqual %bool %58 %float_1
                     OpSelectionMerge %61 None
                     OpBranchConditional %59 %60 %61

         %60 =         OpLabel
                         OpBranch %51

         %61 =     OpLabel
         %62 =       OpAccessChain %_ptr_Function_float %x %int_1
                     OpStore %62 %float_0
                     OpBranch %50

         %50 =     OpLabel
                     OpBranch %51

         %51 =   OpLabel
         %64 =     OpLoad %v4float %x               ; RelaxedPrecision
         %65 =     OpCompositeExtract %float %64 2  ; RelaxedPrecision
         %66 =     OpFOrdGreaterThan %bool %65 %float_0
                   OpBranchConditional %66 %48 %52

         %52 = OpLabel
         %67 =   OpLoad %v4float %x                 ; RelaxedPrecision
                 OpReturnValue %67
               OpFunctionEnd
