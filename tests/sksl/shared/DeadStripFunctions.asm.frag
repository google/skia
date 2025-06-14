               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %9
               OpName %_UniformBuffer "_UniformBuffer"  ; id %14
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %16
               OpName %unpremul_h4h4 "unpremul_h4h4"    ; id %6
               OpName %live_fn_h4h4h4 "live_fn_h4h4h4"  ; id %7
               OpName %main "main"                      ; id %8
               OpName %a "a"                            ; id %56
               OpName %b "b"                            ; id %57

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
               OpDecorate %13 Binding 0
               OpDecorate %13 DescriptorSet 0
               OpDecorate %28 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %a RelaxedPrecision
               OpDecorate %b RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float        ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %13 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %18 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %27 = OpTypeFunction %v4float %_ptr_Function_v4float
    %v3float = OpTypeVector %float 3
%float_9_99999975en05 = OpConstant %float 9.99999975e-05
    %float_1 = OpConstant %float 1
         %46 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
         %53 = OpTypeFunction %v4float %_ptr_Function_v2float
    %float_3 = OpConstant %float 3
         %59 = OpConstantComposite %v4float %float_3 %float_3 %float_3 %float_3
   %float_n5 = OpConstant %float -5
         %62 = OpConstantComposite %v4float %float_n5 %float_n5 %float_n5 %float_n5
         %65 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
         %70 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
     %v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %18

         %19 = OpLabel
         %23 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %23 %22
         %25 =   OpFunctionCall %v4float %main %23
                 OpStore %sk_FragColor %25
                 OpReturn
               OpFunctionEnd


               ; Function unpremul_h4h4
%unpremul_h4h4 = OpFunction %v4float None %27       ; RelaxedPrecision
         %28 = OpFunctionParameter %_ptr_Function_v4float   ; RelaxedPrecision

         %29 = OpLabel
         %30 =   OpLoad %v4float %28                ; RelaxedPrecision
         %31 =   OpVectorShuffle %v3float %30 %30 0 1 2     ; RelaxedPrecision
         %34 =   OpLoad %v4float %28                        ; RelaxedPrecision
         %35 =   OpCompositeExtract %float %34 3            ; RelaxedPrecision
         %33 =   OpExtInst %float %5 FMax %35 %float_9_99999975en05     ; RelaxedPrecision
         %38 =   OpFDiv %float %float_1 %33                             ; RelaxedPrecision
         %39 =   OpVectorTimesScalar %v3float %31 %38                   ; RelaxedPrecision
         %40 =   OpCompositeExtract %float %39 0                        ; RelaxedPrecision
         %41 =   OpCompositeExtract %float %39 1                        ; RelaxedPrecision
         %42 =   OpCompositeExtract %float %39 2                        ; RelaxedPrecision
         %43 =   OpLoad %v4float %28                                    ; RelaxedPrecision
         %44 =   OpCompositeExtract %float %43 3                        ; RelaxedPrecision
         %45 =   OpCompositeConstruct %v4float %40 %41 %42 %44          ; RelaxedPrecision
                 OpReturnValue %45
               OpFunctionEnd


               ; Function live_fn_h4h4h4
%live_fn_h4h4h4 = OpFunction %v4float None %46      ; RelaxedPrecision
         %47 = OpFunctionParameter %_ptr_Function_v4float   ; RelaxedPrecision
         %48 = OpFunctionParameter %_ptr_Function_v4float   ; RelaxedPrecision

         %49 = OpLabel
         %50 =   OpLoad %v4float %47                ; RelaxedPrecision
         %51 =   OpLoad %v4float %48                ; RelaxedPrecision
         %52 =   OpFAdd %v4float %50 %51            ; RelaxedPrecision
                 OpReturnValue %52
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %53         ; RelaxedPrecision
         %54 = OpFunctionParameter %_ptr_Function_v2float

         %55 = OpLabel
          %a =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
          %b =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %60 =   OpVariable %_ptr_Function_v4float Function
         %63 =   OpVariable %_ptr_Function_v4float Function
         %66 =   OpVariable %_ptr_Function_v4float Function
         %79 =   OpVariable %_ptr_Function_v4float Function
                 OpStore %60 %59
                 OpStore %63 %62
         %64 =   OpFunctionCall %v4float %live_fn_h4h4h4 %60 %63
                 OpStore %a %64
                 OpStore %66 %65
         %67 =   OpFunctionCall %v4float %unpremul_h4h4 %66
                 OpStore %b %67
         %71 =   OpFUnordNotEqual %v4bool %64 %70
         %73 =   OpAny %bool %71
                 OpSelectionMerge %75 None
                 OpBranchConditional %73 %74 %75

         %74 =     OpLabel
         %76 =       OpFUnordNotEqual %v4bool %67 %70
         %77 =       OpAny %bool %76
                     OpBranch %75

         %75 = OpLabel
         %78 =   OpPhi %bool %false %55 %77 %74
                 OpSelectionMerge %82 None
                 OpBranchConditional %78 %80 %81

         %80 =     OpLabel
         %83 =       OpAccessChain %_ptr_Uniform_v4float %13 %int_0
         %86 =       OpLoad %v4float %83            ; RelaxedPrecision
                     OpStore %79 %86
                     OpBranch %82

         %81 =     OpLabel
         %87 =       OpAccessChain %_ptr_Uniform_v4float %13 %int_1
         %89 =       OpLoad %v4float %87            ; RelaxedPrecision
                     OpStore %79 %89
                     OpBranch %82

         %82 = OpLabel
         %90 =   OpLoad %v4float %79                ; RelaxedPrecision
                 OpReturnValue %90
               OpFunctionEnd
