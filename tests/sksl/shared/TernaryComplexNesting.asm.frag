               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %4
               OpName %_UniformBuffer "_UniformBuffer"  ; id %9
               OpMemberName %_UniformBuffer 0 "colorWhite"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %11
               OpName %IsEqual_bh4h4 "IsEqual_bh4h4"    ; id %2
               OpName %main "main"                      ; id %3
               OpName %colorBlue "colorBlue"            ; id %35
               OpName %colorGreen "colorGreen"          ; id %45
               OpName %colorRed "colorRed"              ; id %53
               OpName %result "result"                  ; id %61

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %8 Binding 0
               OpDecorate %8 DescriptorSet 0
               OpDecorate %24 RelaxedPrecision
               OpDecorate %25 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %colorBlue RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %colorGreen RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %colorRed RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %result RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision

               ; Types, variables and constants
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float                 ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %8 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %13 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %17 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %23 = OpTypeFunction %bool %_ptr_Function_v4float %_ptr_Function_v4float
     %v4bool = OpTypeVector %bool 4
         %32 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %13

         %14 = OpLabel
         %18 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %18 %17
         %20 =   OpFunctionCall %v4float %main %18
                 OpStore %sk_FragColor %20
                 OpReturn
               OpFunctionEnd


               ; Function IsEqual_bh4h4
%IsEqual_bh4h4 = OpFunction %bool None %23
         %24 = OpFunctionParameter %_ptr_Function_v4float   ; RelaxedPrecision
         %25 = OpFunctionParameter %_ptr_Function_v4float   ; RelaxedPrecision

         %26 = OpLabel
         %27 =   OpLoad %v4float %24                ; RelaxedPrecision
         %28 =   OpLoad %v4float %25                ; RelaxedPrecision
         %29 =   OpFOrdEqual %v4bool %27 %28
         %31 =   OpAll %bool %29
                 OpReturnValue %31
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %32         ; RelaxedPrecision
         %33 = OpFunctionParameter %_ptr_Function_v2float

         %34 = OpLabel
  %colorBlue =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
 %colorGreen =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
   %colorRed =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
     %result =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %65 =   OpVariable %_ptr_Function_v4float Function
         %66 =   OpVariable %_ptr_Function_v4float Function
         %68 =   OpVariable %_ptr_Function_v4float Function
         %72 =   OpVariable %_ptr_Function_v4float Function
         %73 =   OpVariable %_ptr_Function_v4float Function
         %75 =   OpVariable %_ptr_Function_v4float Function
         %81 =   OpVariable %_ptr_Function_v4float Function
         %82 =   OpVariable %_ptr_Function_v4float Function
         %84 =   OpVariable %_ptr_Function_v4float Function
         %92 =   OpVariable %_ptr_Function_v4float Function
         %93 =   OpVariable %_ptr_Function_v4float Function
         %95 =   OpVariable %_ptr_Function_v4float Function
        %102 =   OpVariable %_ptr_Function_v4float Function
        %103 =   OpVariable %_ptr_Function_v4float Function
        %105 =   OpVariable %_ptr_Function_v4float Function
        %109 =   OpVariable %_ptr_Function_v4float Function
        %112 =   OpVariable %_ptr_Function_v4float Function
        %114 =   OpVariable %_ptr_Function_v4float Function
         %36 =   OpAccessChain %_ptr_Uniform_v4float %8 %int_0
         %40 =   OpLoad %v4float %36                ; RelaxedPrecision
         %41 =   OpVectorShuffle %v2float %40 %40 2 3   ; RelaxedPrecision
         %42 =   OpCompositeExtract %float %41 0        ; RelaxedPrecision
         %43 =   OpCompositeExtract %float %41 1        ; RelaxedPrecision
         %44 =   OpCompositeConstruct %v4float %float_0 %float_0 %42 %43    ; RelaxedPrecision
                 OpStore %colorBlue %44
         %46 =   OpAccessChain %_ptr_Uniform_v4float %8 %int_0
         %47 =   OpLoad %v4float %46                ; RelaxedPrecision
         %48 =   OpCompositeExtract %float %47 1    ; RelaxedPrecision
         %49 =   OpAccessChain %_ptr_Uniform_v4float %8 %int_0
         %50 =   OpLoad %v4float %49                ; RelaxedPrecision
         %51 =   OpCompositeExtract %float %50 3    ; RelaxedPrecision
         %52 =   OpCompositeConstruct %v4float %float_0 %48 %float_0 %51    ; RelaxedPrecision
                 OpStore %colorGreen %52
         %54 =   OpAccessChain %_ptr_Uniform_v4float %8 %int_0
         %55 =   OpLoad %v4float %54                ; RelaxedPrecision
         %56 =   OpCompositeExtract %float %55 0    ; RelaxedPrecision
         %57 =   OpAccessChain %_ptr_Uniform_v4float %8 %int_0
         %58 =   OpLoad %v4float %57                ; RelaxedPrecision
         %59 =   OpCompositeExtract %float %58 3    ; RelaxedPrecision
         %60 =   OpCompositeConstruct %v4float %56 %float_0 %float_0 %59    ; RelaxedPrecision
                 OpStore %colorRed %60
         %63 =   OpAccessChain %_ptr_Uniform_v4float %8 %int_0
         %64 =   OpLoad %v4float %63                ; RelaxedPrecision
                 OpStore %65 %64
                 OpStore %66 %44
         %67 =   OpFunctionCall %bool %IsEqual_bh4h4 %65 %66
         %62 =   OpLogicalNot %bool %67
                 OpSelectionMerge %71 None
                 OpBranchConditional %62 %69 %70

         %69 =     OpLabel
                     OpStore %72 %52
                     OpStore %73 %60
         %74 =       OpFunctionCall %bool %IsEqual_bh4h4 %72 %73
                     OpSelectionMerge %78 None
                     OpBranchConditional %74 %76 %77

         %76 =         OpLabel
                         OpStore %75 %60
                         OpBranch %78

         %77 =         OpLabel
                         OpStore %75 %52
                         OpBranch %78

         %78 =     OpLabel
         %79 =       OpLoad %v4float %75            ; RelaxedPrecision
                     OpStore %68 %79
                     OpBranch %71

         %70 =     OpLabel
                     OpStore %81 %60
                     OpStore %82 %52
         %83 =       OpFunctionCall %bool %IsEqual_bh4h4 %81 %82
         %80 =       OpLogicalNot %bool %83
                     OpSelectionMerge %87 None
                     OpBranchConditional %80 %85 %86

         %85 =         OpLabel
                         OpStore %84 %44
                         OpBranch %87

         %86 =         OpLabel
         %88 =           OpAccessChain %_ptr_Uniform_v4float %8 %int_0
         %89 =           OpLoad %v4float %88        ; RelaxedPrecision
                         OpStore %84 %89
                         OpBranch %87

         %87 =     OpLabel
         %90 =       OpLoad %v4float %84            ; RelaxedPrecision
                     OpStore %68 %90
                     OpBranch %71

         %71 = OpLabel
         %91 =   OpLoad %v4float %68                ; RelaxedPrecision
                 OpStore %result %91
                 OpStore %92 %60
                 OpStore %93 %44
         %94 =   OpFunctionCall %bool %IsEqual_bh4h4 %92 %93
                 OpSelectionMerge %98 None
                 OpBranchConditional %94 %96 %97

         %96 =     OpLabel
         %99 =       OpAccessChain %_ptr_Uniform_v4float %8 %int_0
        %100 =       OpLoad %v4float %99            ; RelaxedPrecision
                     OpStore %95 %100
                     OpBranch %98

         %97 =     OpLabel
                     OpStore %102 %60
                     OpStore %103 %52
        %104 =       OpFunctionCall %bool %IsEqual_bh4h4 %102 %103
        %101 =       OpLogicalNot %bool %104
                     OpSelectionMerge %108 None
                     OpBranchConditional %101 %106 %107

        %106 =         OpLabel
                         OpStore %105 %91
                         OpBranch %108

        %107 =         OpLabel
                         OpStore %109 %60
        %110 =           OpAccessChain %_ptr_Uniform_v4float %8 %int_0
        %111 =           OpLoad %v4float %110       ; RelaxedPrecision
                         OpStore %112 %111
        %113 =           OpFunctionCall %bool %IsEqual_bh4h4 %109 %112
                         OpSelectionMerge %117 None
                         OpBranchConditional %113 %115 %116

        %115 =             OpLabel
                             OpStore %114 %44
                             OpBranch %117

        %116 =             OpLabel
                             OpStore %114 %60
                             OpBranch %117

        %117 =         OpLabel
        %118 =           OpLoad %v4float %114       ; RelaxedPrecision
                         OpStore %105 %118
                         OpBranch %108

        %108 =     OpLabel
        %119 =       OpLoad %v4float %105           ; RelaxedPrecision
                     OpStore %95 %119
                     OpBranch %98

         %98 = OpLabel
        %120 =   OpLoad %v4float %95                ; RelaxedPrecision
                 OpReturnValue %120
               OpFunctionEnd
