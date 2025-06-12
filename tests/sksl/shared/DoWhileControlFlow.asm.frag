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

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %x RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision

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
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_1 = OpConstant %float 1
         %23 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Function_float = OpTypePointer Function %float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
 %float_0_25 = OpConstant %float 0.25
       %bool = OpTypeBool
      %int_2 = OpConstant %int 2
      %int_1 = OpConstant %int 1


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
          %x =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
                 OpStore %x %23
                 OpBranch %24

         %24 = OpLabel
                 OpLoopMerge %28 %27 None
                 OpBranch %25

         %25 =     OpLabel
         %29 =       OpAccessChain %_ptr_Function_float %x %int_0
         %33 =       OpLoad %float %29              ; RelaxedPrecision
         %35 =       OpFSub %float %33 %float_0_25  ; RelaxedPrecision
                     OpStore %29 %35
         %36 =       OpLoad %v4float %x             ; RelaxedPrecision
         %37 =       OpCompositeExtract %float %36 0    ; RelaxedPrecision
         %38 =       OpFOrdLessThanEqual %bool %37 %float_0
                     OpSelectionMerge %41 None
                     OpBranchConditional %38 %40 %41

         %40 =         OpLabel
                         OpBranch %28

         %41 =     OpLabel
                     OpBranch %26

         %26 =     OpLabel
                     OpBranch %27

         %27 =   OpLabel
         %42 =     OpLoad %v4float %x               ; RelaxedPrecision
         %43 =     OpCompositeExtract %float %42 3  ; RelaxedPrecision
         %44 =     OpFOrdEqual %bool %43 %float_1
                   OpBranchConditional %44 %24 %28

         %28 = OpLabel
                 OpBranch %45

         %45 = OpLabel
                 OpLoopMerge %49 %48 None
                 OpBranch %46

         %46 =     OpLabel
         %50 =       OpAccessChain %_ptr_Function_float %x %int_2
         %52 =       OpLoad %float %50              ; RelaxedPrecision
         %53 =       OpFSub %float %52 %float_0_25  ; RelaxedPrecision
                     OpStore %50 %53
         %54 =       OpLoad %v4float %x             ; RelaxedPrecision
         %55 =       OpCompositeExtract %float %54 3    ; RelaxedPrecision
         %56 =       OpFOrdEqual %bool %55 %float_1
                     OpSelectionMerge %58 None
                     OpBranchConditional %56 %57 %58

         %57 =         OpLabel
                         OpBranch %48

         %58 =     OpLabel
         %59 =       OpAccessChain %_ptr_Function_float %x %int_1
                     OpStore %59 %float_0
                     OpBranch %47

         %47 =     OpLabel
                     OpBranch %48

         %48 =   OpLabel
         %61 =     OpLoad %v4float %x               ; RelaxedPrecision
         %62 =     OpCompositeExtract %float %61 2  ; RelaxedPrecision
         %63 =     OpFOrdGreaterThan %bool %62 %float_0
                   OpBranchConditional %63 %45 %49

         %49 = OpLabel
         %64 =   OpLoad %v4float %x                 ; RelaxedPrecision
                 OpReturnValue %64
               OpFunctionEnd
