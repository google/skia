               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %5
               OpName %_UniformBuffer "_UniformBuffer"  ; id %10
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %12
               OpName %unpremul_h4h4 "unpremul_h4h4"    ; id %2
               OpName %live_fn_h4h4h4 "live_fn_h4h4h4"  ; id %3
               OpName %main "main"                      ; id %4
               OpName %a "a"                            ; id %52
               OpName %b "b"                            ; id %53

               ; Annotations
               OpDecorate %unpremul_h4h4 RelaxedPrecision
               OpDecorate %live_fn_h4h4h4 RelaxedPrecision
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %9 Binding 0
               OpDecorate %9 DescriptorSet 0
               OpDecorate %24 RelaxedPrecision
               OpDecorate %26 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %a RelaxedPrecision
               OpDecorate %b RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision

               ; Types, variables and constants
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float        ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %9 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %14 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %18 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %23 = OpTypeFunction %v4float %_ptr_Function_v4float
    %v3float = OpTypeVector %float 3
%float_9_99999975en05 = OpConstant %float 9.99999975e-05
    %float_1 = OpConstant %float 1
         %42 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
         %49 = OpTypeFunction %v4float %_ptr_Function_v2float
    %float_3 = OpConstant %float 3
         %55 = OpConstantComposite %v4float %float_3 %float_3 %float_3 %float_3
   %float_n5 = OpConstant %float -5
         %58 = OpConstantComposite %v4float %float_n5 %float_n5 %float_n5 %float_n5
         %61 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
         %66 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
     %v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %14

         %15 = OpLabel
         %19 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %19 %18
         %21 =   OpFunctionCall %v4float %main %19
                 OpStore %sk_FragColor %21
                 OpReturn
               OpFunctionEnd


               ; Function unpremul_h4h4
%unpremul_h4h4 = OpFunction %v4float None %23       ; RelaxedPrecision
         %24 = OpFunctionParameter %_ptr_Function_v4float   ; RelaxedPrecision

         %25 = OpLabel
         %26 =   OpLoad %v4float %24                ; RelaxedPrecision
         %27 =   OpVectorShuffle %v3float %26 %26 0 1 2     ; RelaxedPrecision
         %30 =   OpLoad %v4float %24                        ; RelaxedPrecision
         %31 =   OpCompositeExtract %float %30 3            ; RelaxedPrecision
         %29 =   OpExtInst %float %1 FMax %31 %float_9_99999975en05     ; RelaxedPrecision
         %34 =   OpFDiv %float %float_1 %29                             ; RelaxedPrecision
         %35 =   OpVectorTimesScalar %v3float %27 %34                   ; RelaxedPrecision
         %36 =   OpCompositeExtract %float %35 0                        ; RelaxedPrecision
         %37 =   OpCompositeExtract %float %35 1                        ; RelaxedPrecision
         %38 =   OpCompositeExtract %float %35 2                        ; RelaxedPrecision
         %39 =   OpLoad %v4float %24                                    ; RelaxedPrecision
         %40 =   OpCompositeExtract %float %39 3                        ; RelaxedPrecision
         %41 =   OpCompositeConstruct %v4float %36 %37 %38 %40          ; RelaxedPrecision
                 OpReturnValue %41
               OpFunctionEnd


               ; Function live_fn_h4h4h4
%live_fn_h4h4h4 = OpFunction %v4float None %42      ; RelaxedPrecision
         %43 = OpFunctionParameter %_ptr_Function_v4float   ; RelaxedPrecision
         %44 = OpFunctionParameter %_ptr_Function_v4float   ; RelaxedPrecision

         %45 = OpLabel
         %46 =   OpLoad %v4float %43                ; RelaxedPrecision
         %47 =   OpLoad %v4float %44                ; RelaxedPrecision
         %48 =   OpFAdd %v4float %46 %47            ; RelaxedPrecision
                 OpReturnValue %48
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %49         ; RelaxedPrecision
         %50 = OpFunctionParameter %_ptr_Function_v2float

         %51 = OpLabel
          %a =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
          %b =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %56 =   OpVariable %_ptr_Function_v4float Function
         %59 =   OpVariable %_ptr_Function_v4float Function
         %62 =   OpVariable %_ptr_Function_v4float Function
         %75 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %56 %55
                 OpStore %59 %58
         %60 =   OpFunctionCall %v4float %live_fn_h4h4h4 %56 %59
                 OpStore %a %60
                 OpStore %62 %61
         %63 =   OpFunctionCall %v4float %unpremul_h4h4 %62
                 OpStore %b %63
         %67 =   OpFUnordNotEqual %v4bool %60 %66
         %69 =   OpAny %bool %67
                 OpSelectionMerge %71 None
                 OpBranchConditional %69 %70 %71

         %70 =     OpLabel
         %72 =       OpFUnordNotEqual %v4bool %63 %66
         %73 =       OpAny %bool %72
                     OpBranch %71

         %71 = OpLabel
         %74 =   OpPhi %bool %false %51 %73 %70
                 OpSelectionMerge %78 None
                 OpBranchConditional %74 %76 %77

         %76 =     OpLabel
         %79 =       OpAccessChain %_ptr_Uniform_v4float %9 %int_0
         %83 =       OpLoad %v4float %79            ; RelaxedPrecision
                     OpStore %75 %83
                     OpBranch %78

         %77 =     OpLabel
         %84 =       OpAccessChain %_ptr_Uniform_v4float %9 %int_1
         %86 =       OpLoad %v4float %84            ; RelaxedPrecision
                     OpStore %75 %86
                     OpBranch %78

         %78 = OpLabel
         %87 =   OpLoad %v4float %75                ; RelaxedPrecision
                 OpReturnValue %87
               OpFunctionEnd
