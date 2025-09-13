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
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
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
       %bool = OpTypeBool
%_ptr_Function_float = OpTypePointer Function %float
      %int_0 = OpConstant %int 0
 %float_0_25 = OpConstant %float 0.25
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
         %33 =       OpLoad %v4float %x             ; RelaxedPrecision
         %34 =       OpCompositeExtract %float %33 3    ; RelaxedPrecision
         %35 =       OpFOrdEqual %bool %34 %float_1
                     OpBranchConditional %35 %30 %32

         %30 =         OpLabel
         %37 =           OpAccessChain %_ptr_Function_float %x %int_0
         %40 =           OpLoad %float %37          ; RelaxedPrecision
         %42 =           OpFSub %float %40 %float_0_25  ; RelaxedPrecision
                         OpStore %37 %42
         %43 =           OpLoad %v4float %x         ; RelaxedPrecision
         %44 =           OpCompositeExtract %float %43 0    ; RelaxedPrecision
         %45 =           OpFOrdLessThanEqual %bool %44 %float_0
                         OpSelectionMerge %47 None
                         OpBranchConditional %45 %46 %47

         %46 =             OpLabel
                             OpBranch %32

         %47 =         OpLabel
                         OpBranch %31

         %31 =   OpLabel
                   OpBranch %28

         %32 = OpLabel
                 OpBranch %48

         %48 = OpLabel
                 OpLoopMerge %52 %51 None
                 OpBranch %49

         %49 =     OpLabel
         %53 =       OpLoad %v4float %x             ; RelaxedPrecision
         %54 =       OpCompositeExtract %float %53 2    ; RelaxedPrecision
         %55 =       OpFOrdGreaterThan %bool %54 %float_0
                     OpBranchConditional %55 %50 %52

         %50 =         OpLabel
         %56 =           OpAccessChain %_ptr_Function_float %x %int_2
         %58 =           OpLoad %float %56          ; RelaxedPrecision
         %59 =           OpFSub %float %58 %float_0_25  ; RelaxedPrecision
                         OpStore %56 %59
         %60 =           OpLoad %v4float %x         ; RelaxedPrecision
         %61 =           OpCompositeExtract %float %60 3    ; RelaxedPrecision
         %62 =           OpFOrdEqual %bool %61 %float_1
                         OpSelectionMerge %64 None
                         OpBranchConditional %62 %63 %64

         %63 =             OpLabel
                             OpBranch %51

         %64 =         OpLabel
         %65 =           OpAccessChain %_ptr_Function_float %x %int_1
                         OpStore %65 %float_0
                         OpBranch %51

         %51 =   OpLabel
                   OpBranch %48

         %52 = OpLabel
         %67 =   OpLoad %v4float %x                 ; RelaxedPrecision
                 OpReturnValue %67
               OpFunctionEnd
