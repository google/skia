               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %8
               OpName %_UniformBuffer "_UniformBuffer"  ; id %13
               OpMemberName %_UniformBuffer 0 "colorWhite"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %15
               OpName %IsEqual_bh4h4 "IsEqual_bh4h4"    ; id %6
               OpName %main "main"                      ; id %7
               OpName %colorBlue "colorBlue"            ; id %39
               OpName %colorGreen "colorGreen"          ; id %48
               OpName %colorRed "colorRed"              ; id %56
               OpName %result "result"                  ; id %64

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %12 Binding 0
               OpDecorate %12 DescriptorSet 0
               OpDecorate %28 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %colorBlue RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %colorGreen RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %colorRed RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %result RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float                 ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %17 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %27 = OpTypeFunction %bool %_ptr_Function_v4float %_ptr_Function_v4float
     %v4bool = OpTypeVector %bool 4
         %36 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %17

         %18 = OpLabel
         %22 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %22 %21
         %24 =   OpFunctionCall %v4float %main %22
                 OpStore %sk_FragColor %24
                 OpReturn
               OpFunctionEnd


               ; Function IsEqual_bh4h4
%IsEqual_bh4h4 = OpFunction %bool None %27
         %28 = OpFunctionParameter %_ptr_Function_v4float   ; RelaxedPrecision
         %29 = OpFunctionParameter %_ptr_Function_v4float   ; RelaxedPrecision

         %30 = OpLabel
         %31 =   OpLoad %v4float %28                ; RelaxedPrecision
         %32 =   OpLoad %v4float %29                ; RelaxedPrecision
         %33 =   OpFOrdEqual %v4bool %31 %32
         %35 =   OpAll %bool %33
                 OpReturnValue %35
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %36         ; RelaxedPrecision
         %37 = OpFunctionParameter %_ptr_Function_v2float

         %38 = OpLabel
  %colorBlue =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
 %colorGreen =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
   %colorRed =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
     %result =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %68 =   OpVariable %_ptr_Function_v4float Function
         %69 =   OpVariable %_ptr_Function_v4float Function
         %71 =   OpVariable %_ptr_Function_v4float Function
         %75 =   OpVariable %_ptr_Function_v4float Function
         %76 =   OpVariable %_ptr_Function_v4float Function
         %78 =   OpVariable %_ptr_Function_v4float Function
         %84 =   OpVariable %_ptr_Function_v4float Function
         %85 =   OpVariable %_ptr_Function_v4float Function
         %87 =   OpVariable %_ptr_Function_v4float Function
         %95 =   OpVariable %_ptr_Function_v4float Function
         %96 =   OpVariable %_ptr_Function_v4float Function
         %98 =   OpVariable %_ptr_Function_v4float Function
        %105 =   OpVariable %_ptr_Function_v4float Function
        %106 =   OpVariable %_ptr_Function_v4float Function
        %108 =   OpVariable %_ptr_Function_v4float Function
        %112 =   OpVariable %_ptr_Function_v4float Function
        %115 =   OpVariable %_ptr_Function_v4float Function
        %117 =   OpVariable %_ptr_Function_v4float Function
         %40 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %43 =   OpLoad %v4float %40                ; RelaxedPrecision
         %44 =   OpVectorShuffle %v2float %43 %43 2 3   ; RelaxedPrecision
         %45 =   OpCompositeExtract %float %44 0        ; RelaxedPrecision
         %46 =   OpCompositeExtract %float %44 1        ; RelaxedPrecision
         %47 =   OpCompositeConstruct %v4float %float_0 %float_0 %45 %46    ; RelaxedPrecision
                 OpStore %colorBlue %47
         %49 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %50 =   OpLoad %v4float %49                ; RelaxedPrecision
         %51 =   OpCompositeExtract %float %50 1    ; RelaxedPrecision
         %52 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %53 =   OpLoad %v4float %52                ; RelaxedPrecision
         %54 =   OpCompositeExtract %float %53 3    ; RelaxedPrecision
         %55 =   OpCompositeConstruct %v4float %float_0 %51 %float_0 %54    ; RelaxedPrecision
                 OpStore %colorGreen %55
         %57 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %58 =   OpLoad %v4float %57                ; RelaxedPrecision
         %59 =   OpCompositeExtract %float %58 0    ; RelaxedPrecision
         %60 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %61 =   OpLoad %v4float %60                ; RelaxedPrecision
         %62 =   OpCompositeExtract %float %61 3    ; RelaxedPrecision
         %63 =   OpCompositeConstruct %v4float %59 %float_0 %float_0 %62    ; RelaxedPrecision
                 OpStore %colorRed %63
         %66 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %67 =   OpLoad %v4float %66                ; RelaxedPrecision
                 OpStore %68 %67
                 OpStore %69 %47
         %70 =   OpFunctionCall %bool %IsEqual_bh4h4 %68 %69
         %65 =   OpLogicalNot %bool %70
                 OpSelectionMerge %74 None
                 OpBranchConditional %65 %72 %73

         %72 =     OpLabel
                     OpStore %75 %55
                     OpStore %76 %63
         %77 =       OpFunctionCall %bool %IsEqual_bh4h4 %75 %76
                     OpSelectionMerge %81 None
                     OpBranchConditional %77 %79 %80

         %79 =         OpLabel
                         OpStore %78 %63
                         OpBranch %81

         %80 =         OpLabel
                         OpStore %78 %55
                         OpBranch %81

         %81 =     OpLabel
         %82 =       OpLoad %v4float %78            ; RelaxedPrecision
                     OpStore %71 %82
                     OpBranch %74

         %73 =     OpLabel
                     OpStore %84 %63
                     OpStore %85 %55
         %86 =       OpFunctionCall %bool %IsEqual_bh4h4 %84 %85
         %83 =       OpLogicalNot %bool %86
                     OpSelectionMerge %90 None
                     OpBranchConditional %83 %88 %89

         %88 =         OpLabel
                         OpStore %87 %47
                         OpBranch %90

         %89 =         OpLabel
         %91 =           OpAccessChain %_ptr_Uniform_v4float %12 %int_0
         %92 =           OpLoad %v4float %91        ; RelaxedPrecision
                         OpStore %87 %92
                         OpBranch %90

         %90 =     OpLabel
         %93 =       OpLoad %v4float %87            ; RelaxedPrecision
                     OpStore %71 %93
                     OpBranch %74

         %74 = OpLabel
         %94 =   OpLoad %v4float %71                ; RelaxedPrecision
                 OpStore %result %94
                 OpStore %95 %63
                 OpStore %96 %47
         %97 =   OpFunctionCall %bool %IsEqual_bh4h4 %95 %96
                 OpSelectionMerge %101 None
                 OpBranchConditional %97 %99 %100

         %99 =     OpLabel
        %102 =       OpAccessChain %_ptr_Uniform_v4float %12 %int_0
        %103 =       OpLoad %v4float %102           ; RelaxedPrecision
                     OpStore %98 %103
                     OpBranch %101

        %100 =     OpLabel
                     OpStore %105 %63
                     OpStore %106 %55
        %107 =       OpFunctionCall %bool %IsEqual_bh4h4 %105 %106
        %104 =       OpLogicalNot %bool %107
                     OpSelectionMerge %111 None
                     OpBranchConditional %104 %109 %110

        %109 =         OpLabel
                         OpStore %108 %94
                         OpBranch %111

        %110 =         OpLabel
                         OpStore %112 %63
        %113 =           OpAccessChain %_ptr_Uniform_v4float %12 %int_0
        %114 =           OpLoad %v4float %113       ; RelaxedPrecision
                         OpStore %115 %114
        %116 =           OpFunctionCall %bool %IsEqual_bh4h4 %112 %115
                         OpSelectionMerge %120 None
                         OpBranchConditional %116 %118 %119

        %118 =             OpLabel
                             OpStore %117 %47
                             OpBranch %120

        %119 =             OpLabel
                             OpStore %117 %63
                             OpBranch %120

        %120 =         OpLabel
        %121 =           OpLoad %v4float %117       ; RelaxedPrecision
                         OpStore %108 %121
                         OpBranch %111

        %111 =     OpLabel
        %122 =       OpLoad %v4float %108           ; RelaxedPrecision
                     OpStore %98 %122
                     OpBranch %101

        %101 = OpLabel
        %123 =   OpLoad %v4float %98                ; RelaxedPrecision
                 OpReturnValue %123
               OpFunctionEnd
