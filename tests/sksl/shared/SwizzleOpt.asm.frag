               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %8
               OpName %_UniformBuffer "_UniformBuffer"  ; id %13
               OpMemberName %_UniformBuffer 0 "colorRed"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "testInputs"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %15
               OpName %fn_hh4 "fn_hh4"                  ; id %6
               OpName %x "x"                            ; id %29
               OpName %main "main"                      ; id %7
               OpName %v "v"                            ; id %48

               ; Annotations
               OpDecorate %fn_hh4 RelaxedPrecision
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %12 Binding 0
               OpDecorate %12 DescriptorSet 0
               OpDecorate %27 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %v RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %125 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
               OpDecorate %129 RelaxedPrecision
               OpDecorate %130 RelaxedPrecision
               OpDecorate %140 RelaxedPrecision
               OpDecorate %142 RelaxedPrecision
               OpDecorate %143 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float   ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %17 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %26 = OpTypeFunction %float %_ptr_Function_v4float
%_ptr_Function_int = OpTypePointer Function %int
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
       %bool = OpTypeBool
         %45 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
    %v3float = OpTypeVector %float 3
    %float_1 = OpConstant %float 1
  %float_123 = OpConstant %float 123
  %float_456 = OpConstant %float 456
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
        %102 = OpConstantComposite %v4float %float_1 %float_1 %float_2 %float_3
      %int_0 = OpConstant %int 0
        %131 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
     %v4bool = OpTypeVector %bool 4


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %17

         %18 = OpLabel
         %22 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %22 %21
         %24 =   OpFunctionCall %v4float %main %22
                 OpStore %sk_FragColor %24
                 OpReturn
               OpFunctionEnd


               ; Function fn_hh4
     %fn_hh4 = OpFunction %float None %26           ; RelaxedPrecision
         %27 = OpFunctionParameter %_ptr_Function_v4float   ; RelaxedPrecision

         %28 = OpLabel
          %x =   OpVariable %_ptr_Function_int Function
                 OpStore %x %int_1
                 OpBranch %32

         %32 = OpLabel
                 OpLoopMerge %36 %35 None
                 OpBranch %33

         %33 =     OpLabel
         %37 =       OpLoad %int %x
         %39 =       OpSLessThanEqual %bool %37 %int_2
                     OpBranchConditional %39 %34 %36

         %34 =         OpLabel
         %41 =           OpLoad %v4float %27        ; RelaxedPrecision
         %42 =           OpCompositeExtract %float %41 0    ; RelaxedPrecision
                         OpReturnValue %42

         %35 =   OpLabel
         %43 =     OpLoad %int %x
         %44 =     OpIAdd %int %43 %int_1
                   OpStore %x %44
                   OpBranch %32

         %36 = OpLabel
                 OpUnreachable
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %45         ; RelaxedPrecision
         %46 = OpFunctionParameter %_ptr_Function_v2float

         %47 = OpLabel
          %v =   OpVariable %_ptr_Function_v4float Function     ; RelaxedPrecision
         %76 =   OpVariable %_ptr_Function_v4float Function
         %82 =   OpVariable %_ptr_Function_v4float Function
         %86 =   OpVariable %_ptr_Function_v4float Function
         %89 =   OpVariable %_ptr_Function_v4float Function
         %92 =   OpVariable %_ptr_Function_v4float Function
         %96 =   OpVariable %_ptr_Function_v4float Function
        %135 =   OpVariable %_ptr_Function_v4float Function
         %49 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_2
         %51 =   OpLoad %v4float %49                ; RelaxedPrecision
                 OpStore %v %51
         %52 =   OpVectorShuffle %v3float %51 %51 2 1 0     ; RelaxedPrecision
         %54 =   OpCompositeExtract %float %52 0            ; RelaxedPrecision
         %55 =   OpCompositeExtract %float %52 1            ; RelaxedPrecision
         %56 =   OpCompositeExtract %float %52 2            ; RelaxedPrecision
         %57 =   OpCompositeConstruct %v4float %float_0 %54 %55 %56     ; RelaxedPrecision
                 OpStore %v %57
         %58 =   OpVectorShuffle %v2float %57 %57 0 3   ; RelaxedPrecision
         %59 =   OpCompositeExtract %float %58 0        ; RelaxedPrecision
         %60 =   OpCompositeExtract %float %58 1        ; RelaxedPrecision
         %61 =   OpCompositeConstruct %v4float %float_0 %float_0 %59 %60    ; RelaxedPrecision
                 OpStore %v %61
         %63 =   OpVectorShuffle %v2float %61 %61 3 0   ; RelaxedPrecision
         %64 =   OpCompositeExtract %float %63 0        ; RelaxedPrecision
         %65 =   OpCompositeExtract %float %63 1        ; RelaxedPrecision
         %66 =   OpCompositeConstruct %v4float %float_1 %float_1 %64 %65    ; RelaxedPrecision
                 OpStore %v %66
         %67 =   OpVectorShuffle %v2float %66 %66 2 1   ; RelaxedPrecision
         %68 =   OpCompositeExtract %float %67 0        ; RelaxedPrecision
         %69 =   OpCompositeExtract %float %67 1        ; RelaxedPrecision
         %70 =   OpCompositeConstruct %v4float %68 %69 %float_1 %float_1    ; RelaxedPrecision
                 OpStore %v %70
         %71 =   OpVectorShuffle %v2float %70 %70 0 0   ; RelaxedPrecision
         %72 =   OpCompositeExtract %float %71 0        ; RelaxedPrecision
         %73 =   OpCompositeExtract %float %71 1        ; RelaxedPrecision
         %74 =   OpCompositeConstruct %v4float %72 %73 %float_1 %float_1    ; RelaxedPrecision
                 OpStore %v %74
         %75 =   OpVectorShuffle %v4float %74 %74 3 2 3 2   ; RelaxedPrecision
                 OpStore %v %75
                 OpStore %76 %75
         %77 =   OpFunctionCall %float %fn_hh4 %76
         %80 =   OpCompositeConstruct %v3float %77 %float_123 %float_456    ; RelaxedPrecision
         %81 =   OpVectorShuffle %v4float %80 %80 1 1 2 2                   ; RelaxedPrecision
                 OpStore %v %81
                 OpStore %82 %81
         %83 =   OpFunctionCall %float %fn_hh4 %82
         %84 =   OpCompositeConstruct %v3float %83 %float_123 %float_456    ; RelaxedPrecision
         %85 =   OpVectorShuffle %v4float %84 %84 1 1 2 2                   ; RelaxedPrecision
                 OpStore %v %85
                 OpStore %86 %85
         %87 =   OpFunctionCall %float %fn_hh4 %86
         %88 =   OpCompositeConstruct %v4float %float_123 %float_456 %float_456 %87     ; RelaxedPrecision
                 OpStore %v %88
                 OpStore %89 %88
         %90 =   OpFunctionCall %float %fn_hh4 %89
         %91 =   OpCompositeConstruct %v4float %float_123 %float_456 %float_456 %90     ; RelaxedPrecision
                 OpStore %v %91
                 OpStore %92 %91
         %93 =   OpFunctionCall %float %fn_hh4 %92
         %94 =   OpCompositeConstruct %v3float %93 %float_123 %float_456    ; RelaxedPrecision
         %95 =   OpVectorShuffle %v4float %94 %94 1 0 0 2                   ; RelaxedPrecision
                 OpStore %v %95
                 OpStore %96 %95
         %97 =   OpFunctionCall %float %fn_hh4 %96
         %98 =   OpCompositeConstruct %v3float %97 %float_123 %float_456    ; RelaxedPrecision
         %99 =   OpVectorShuffle %v4float %98 %98 1 0 0 2                   ; RelaxedPrecision
                 OpStore %v %99
                 OpStore %v %102
        %103 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_0
        %105 =   OpLoad %v4float %103               ; RelaxedPrecision
        %106 =   OpVectorShuffle %v3float %105 %105 0 1 2   ; RelaxedPrecision
        %107 =   OpCompositeExtract %float %106 0           ; RelaxedPrecision
        %108 =   OpCompositeExtract %float %106 1           ; RelaxedPrecision
        %109 =   OpCompositeExtract %float %106 2           ; RelaxedPrecision
        %110 =   OpCompositeConstruct %v4float %107 %108 %109 %float_1  ; RelaxedPrecision
                 OpStore %v %110
        %111 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_0
        %112 =   OpLoad %v4float %111               ; RelaxedPrecision
        %113 =   OpCompositeExtract %float %112 0   ; RelaxedPrecision
        %114 =   OpAccessChain %_ptr_Uniform_v4float %12 %int_0
        %115 =   OpLoad %v4float %114               ; RelaxedPrecision
        %116 =   OpVectorShuffle %v2float %115 %115 1 2     ; RelaxedPrecision
        %117 =   OpCompositeExtract %float %116 0           ; RelaxedPrecision
        %118 =   OpCompositeExtract %float %116 1           ; RelaxedPrecision
        %119 =   OpCompositeConstruct %v4float %113 %float_1 %117 %118  ; RelaxedPrecision
                 OpStore %v %119
        %120 =   OpLoad %v4float %v                 ; RelaxedPrecision
        %121 =   OpVectorShuffle %v4float %120 %119 7 6 5 4     ; RelaxedPrecision
                 OpStore %v %121
        %122 =   OpVectorShuffle %v2float %121 %121 1 2     ; RelaxedPrecision
        %123 =   OpLoad %v4float %v                         ; RelaxedPrecision
        %124 =   OpVectorShuffle %v4float %123 %122 4 1 2 5     ; RelaxedPrecision
                 OpStore %v %124
        %125 =   OpVectorShuffle %v2float %124 %124 3 3     ; RelaxedPrecision
        %126 =   OpCompositeExtract %float %125 0           ; RelaxedPrecision
        %127 =   OpCompositeExtract %float %125 1           ; RelaxedPrecision
        %128 =   OpCompositeConstruct %v3float %126 %127 %float_1   ; RelaxedPrecision
        %129 =   OpLoad %v4float %v                                 ; RelaxedPrecision
        %130 =   OpVectorShuffle %v4float %129 %128 6 5 4 3         ; RelaxedPrecision
                 OpStore %v %130
        %132 =   OpFOrdEqual %v4bool %130 %131
        %134 =   OpAll %bool %132
                 OpSelectionMerge %138 None
                 OpBranchConditional %134 %136 %137

        %136 =     OpLabel
        %139 =       OpAccessChain %_ptr_Uniform_v4float %12 %int_1
        %140 =       OpLoad %v4float %139           ; RelaxedPrecision
                     OpStore %135 %140
                     OpBranch %138

        %137 =     OpLabel
        %141 =       OpAccessChain %_ptr_Uniform_v4float %12 %int_0
        %142 =       OpLoad %v4float %141           ; RelaxedPrecision
                     OpStore %135 %142
                     OpBranch %138

        %138 = OpLabel
        %143 =   OpLoad %v4float %135               ; RelaxedPrecision
                 OpReturnValue %143
               OpFunctionEnd
