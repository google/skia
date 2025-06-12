               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %3
               OpName %_UniformBuffer "_UniformBuffer"  ; id %8
               OpMemberName %_UniformBuffer 0 "colorWhite"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %10
               OpName %main "main"                      ; id %2
               OpName %colorBlue "colorBlue"            ; id %23
               OpName %colorGreen "colorGreen"          ; id %34
               OpName %colorRed "colorRed"              ; id %42
               OpName %result "result"                  ; id %50

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %colorBlue RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %colorGreen RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %colorRed RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %result RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision

               ; Types, variables and constants
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float                 ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %bool = OpTypeBool
     %v4bool = OpTypeVector %bool 4


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %12

         %13 = OpLabel
         %17 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %17 %16
         %19 =   OpFunctionCall %v4float %main %17
                 OpStore %sk_FragColor %19
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %20         ; RelaxedPrecision
         %21 = OpFunctionParameter %_ptr_Function_v2float

         %22 = OpLabel
  %colorBlue =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
 %colorGreen =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
   %colorRed =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
     %result =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %57 =   OpVariable %_ptr_Function_v4float Function
         %63 =   OpVariable %_ptr_Function_v4float Function
         %70 =   OpVariable %_ptr_Function_v4float Function
         %80 =   OpVariable %_ptr_Function_v4float Function
         %88 =   OpVariable %_ptr_Function_v4float Function
         %96 =   OpVariable %_ptr_Function_v4float Function
         %25 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %29 =   OpLoad %v4float %25                ; RelaxedPrecision
         %30 =   OpVectorShuffle %v2float %29 %29 2 3   ; RelaxedPrecision
         %31 =   OpCompositeExtract %float %30 0        ; RelaxedPrecision
         %32 =   OpCompositeExtract %float %30 1        ; RelaxedPrecision
         %33 =   OpCompositeConstruct %v4float %float_0 %float_0 %31 %32    ; RelaxedPrecision
                 OpStore %colorBlue %33
         %35 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %36 =   OpLoad %v4float %35                ; RelaxedPrecision
         %37 =   OpCompositeExtract %float %36 1    ; RelaxedPrecision
         %38 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %39 =   OpLoad %v4float %38                ; RelaxedPrecision
         %40 =   OpCompositeExtract %float %39 3    ; RelaxedPrecision
         %41 =   OpCompositeConstruct %v4float %float_0 %37 %float_0 %40    ; RelaxedPrecision
                 OpStore %colorGreen %41
         %43 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %44 =   OpLoad %v4float %43                ; RelaxedPrecision
         %45 =   OpCompositeExtract %float %44 0    ; RelaxedPrecision
         %46 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %47 =   OpLoad %v4float %46                ; RelaxedPrecision
         %48 =   OpCompositeExtract %float %47 3    ; RelaxedPrecision
         %49 =   OpCompositeConstruct %v4float %45 %float_0 %float_0 %48    ; RelaxedPrecision
                 OpStore %colorRed %49
         %51 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %52 =   OpLoad %v4float %51                ; RelaxedPrecision
         %53 =   OpFUnordNotEqual %v4bool %52 %33
         %56 =   OpAny %bool %53
                 OpSelectionMerge %60 None
                 OpBranchConditional %56 %58 %59

         %58 =     OpLabel
         %61 =       OpFOrdEqual %v4bool %41 %49
         %62 =       OpAll %bool %61
                     OpSelectionMerge %66 None
                     OpBranchConditional %62 %64 %65

         %64 =         OpLabel
                         OpStore %63 %49
                         OpBranch %66

         %65 =         OpLabel
                         OpStore %63 %41
                         OpBranch %66

         %66 =     OpLabel
         %67 =       OpLoad %v4float %63            ; RelaxedPrecision
                     OpStore %57 %67
                     OpBranch %60

         %59 =     OpLabel
         %68 =       OpFUnordNotEqual %v4bool %49 %41
         %69 =       OpAny %bool %68
                     OpSelectionMerge %73 None
                     OpBranchConditional %69 %71 %72

         %71 =         OpLabel
                         OpStore %70 %33
                         OpBranch %73

         %72 =         OpLabel
         %74 =           OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %75 =           OpLoad %v4float %74        ; RelaxedPrecision
                         OpStore %70 %75
                         OpBranch %73

         %73 =     OpLabel
         %76 =       OpLoad %v4float %70            ; RelaxedPrecision
                     OpStore %57 %76
                     OpBranch %60

         %60 = OpLabel
         %77 =   OpLoad %v4float %57                ; RelaxedPrecision
                 OpStore %result %77
         %78 =   OpFOrdEqual %v4bool %49 %33
         %79 =   OpAll %bool %78
                 OpSelectionMerge %83 None
                 OpBranchConditional %79 %81 %82

         %81 =     OpLabel
         %84 =       OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %85 =       OpLoad %v4float %84            ; RelaxedPrecision
                     OpStore %80 %85
                     OpBranch %83

         %82 =     OpLabel
         %86 =       OpFUnordNotEqual %v4bool %49 %41
         %87 =       OpAny %bool %86
                     OpSelectionMerge %91 None
                     OpBranchConditional %87 %89 %90

         %89 =         OpLabel
                         OpStore %88 %77
                         OpBranch %91

         %90 =         OpLabel
         %92 =           OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %93 =           OpLoad %v4float %92        ; RelaxedPrecision
         %94 =           OpFOrdEqual %v4bool %49 %93
         %95 =           OpAll %bool %94
                         OpSelectionMerge %99 None
                         OpBranchConditional %95 %97 %98

         %97 =             OpLabel
                             OpStore %96 %33
                             OpBranch %99

         %98 =             OpLabel
                             OpStore %96 %49
                             OpBranch %99

         %99 =         OpLabel
        %100 =           OpLoad %v4float %96        ; RelaxedPrecision
                         OpStore %88 %100
                         OpBranch %91

         %91 =     OpLabel
        %101 =       OpLoad %v4float %88            ; RelaxedPrecision
                     OpStore %80 %101
                     OpBranch %83

         %83 = OpLabel
        %102 =   OpLoad %v4float %80                ; RelaxedPrecision
                 OpReturnValue %102
               OpFunctionEnd
