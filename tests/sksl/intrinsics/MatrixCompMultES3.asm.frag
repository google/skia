               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %12
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %14
               OpName %main "main"                      ; id %6
               OpName %h24 "h24"                        ; id %27
               OpName %h42 "h42"                        ; id %44
               OpName %f43 "f43"                        ; id %82

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %h24 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %h42 RelaxedPrecision
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
               OpDecorate %76 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %149 RelaxedPrecision
               OpDecorate %151 RelaxedPrecision
               OpDecorate %152 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%_UniformBuffer = OpTypeStruct %v4float %v4float        ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %24 = OpTypeFunction %v4float %_ptr_Function_v2float
%mat2v4float = OpTypeMatrix %v4float 2
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
    %float_9 = OpConstant %float 9
         %31 = OpConstantComposite %v4float %float_9 %float_9 %float_9 %float_9
         %32 = OpConstantComposite %mat2v4float %31 %31
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
%mat4v2float = OpTypeMatrix %v2float 4
%_ptr_Function_mat4v2float = OpTypePointer Function %mat4v2float
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
    %float_5 = OpConstant %float 5
    %float_6 = OpConstant %float 6
    %float_7 = OpConstant %float 7
    %float_8 = OpConstant %float 8
         %55 = OpConstantComposite %v2float %float_1 %float_2
         %56 = OpConstantComposite %v2float %float_3 %float_4
         %57 = OpConstantComposite %v2float %float_5 %float_6
         %58 = OpConstantComposite %v2float %float_7 %float_8
         %59 = OpConstantComposite %mat4v2float %55 %56 %57 %58
    %v3float = OpTypeVector %float 3
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
   %float_12 = OpConstant %float 12
   %float_22 = OpConstant %float 22
   %float_30 = OpConstant %float 30
   %float_36 = OpConstant %float 36
   %float_40 = OpConstant %float 40
   %float_42 = OpConstant %float 42
         %92 = OpConstantComposite %v3float %float_12 %float_22 %float_30
         %93 = OpConstantComposite %v3float %float_36 %float_40 %float_42
         %94 = OpConstantComposite %v3float %float_42 %float_40 %float_36
         %95 = OpConstantComposite %v3float %float_30 %float_22 %float_12
         %96 = OpConstantComposite %mat4v3float %92 %93 %94 %95
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
         %99 = OpConstantComposite %v4float %float_9 %float_0 %float_0 %float_9
        %100 = OpConstantComposite %v4float %float_0 %float_9 %float_0 %float_9
        %101 = OpConstantComposite %mat2v4float %99 %100
     %v4bool = OpTypeVector %bool 4
        %110 = OpConstantComposite %v2float %float_1 %float_0
        %111 = OpConstantComposite %v2float %float_0 %float_4
        %112 = OpConstantComposite %v2float %float_0 %float_6
        %113 = OpConstantComposite %v2float %float_0 %float_8
        %114 = OpConstantComposite %mat4v2float %110 %111 %112 %113
     %v2bool = OpTypeVector %bool 2
     %v3bool = OpTypeVector %bool 3
%_ptr_Function_v4float = OpTypePointer Function %v4float


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %16

         %17 = OpLabel
         %21 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %21 %20
         %23 =   OpFunctionCall %v4float %main %21
                 OpStore %sk_FragColor %23
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %24         ; RelaxedPrecision
         %25 = OpFunctionParameter %_ptr_Function_v2float

         %26 = OpLabel
        %h24 =   OpVariable %_ptr_Function_mat2v4float Function     ; RelaxedPrecision
        %h42 =   OpVariable %_ptr_Function_mat4v2float Function     ; RelaxedPrecision
        %f43 =   OpVariable %_ptr_Function_mat4v3float Function
        %143 =   OpVariable %_ptr_Function_v4float Function
         %33 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %36 =   OpLoad %v4float %33                ; RelaxedPrecision
         %37 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %39 =   OpLoad %v4float %37                ; RelaxedPrecision
         %40 =   OpCompositeConstruct %mat2v4float %36 %39  ; RelaxedPrecision
         %41 =   OpFMul %v4float %31 %36                    ; RelaxedPrecision
         %42 =   OpFMul %v4float %31 %39                    ; RelaxedPrecision
         %43 =   OpCompositeConstruct %mat2v4float %41 %42  ; RelaxedPrecision
                 OpStore %h24 %43
         %60 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %61 =   OpLoad %v4float %60                ; RelaxedPrecision
         %62 =   OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %63 =   OpLoad %v4float %62                ; RelaxedPrecision
         %64 =   OpCompositeExtract %float %61 0    ; RelaxedPrecision
         %65 =   OpCompositeExtract %float %61 1    ; RelaxedPrecision
         %66 =   OpCompositeConstruct %v2float %64 %65  ; RelaxedPrecision
         %67 =   OpCompositeExtract %float %61 2        ; RelaxedPrecision
         %68 =   OpCompositeExtract %float %61 3        ; RelaxedPrecision
         %69 =   OpCompositeConstruct %v2float %67 %68  ; RelaxedPrecision
         %70 =   OpCompositeExtract %float %63 0        ; RelaxedPrecision
         %71 =   OpCompositeExtract %float %63 1        ; RelaxedPrecision
         %72 =   OpCompositeConstruct %v2float %70 %71  ; RelaxedPrecision
         %73 =   OpCompositeExtract %float %63 2        ; RelaxedPrecision
         %74 =   OpCompositeExtract %float %63 3        ; RelaxedPrecision
         %75 =   OpCompositeConstruct %v2float %73 %74  ; RelaxedPrecision
         %76 =   OpCompositeConstruct %mat4v2float %66 %69 %72 %75  ; RelaxedPrecision
         %77 =   OpFMul %v2float %55 %66                            ; RelaxedPrecision
         %78 =   OpFMul %v2float %56 %69                            ; RelaxedPrecision
         %79 =   OpFMul %v2float %57 %72                            ; RelaxedPrecision
         %80 =   OpFMul %v2float %58 %75                            ; RelaxedPrecision
         %81 =   OpCompositeConstruct %mat4v2float %77 %78 %79 %80  ; RelaxedPrecision
                 OpStore %h42 %81
                 OpStore %f43 %96
        %103 =   OpFOrdEqual %v4bool %41 %99        ; RelaxedPrecision
        %104 =   OpAll %bool %103
        %105 =   OpFOrdEqual %v4bool %42 %100       ; RelaxedPrecision
        %106 =   OpAll %bool %105
        %107 =   OpLogicalAnd %bool %104 %106
                 OpSelectionMerge %109 None
                 OpBranchConditional %107 %108 %109

        %108 =     OpLabel
        %116 =       OpFOrdEqual %v2bool %77 %110   ; RelaxedPrecision
        %117 =       OpAll %bool %116
        %118 =       OpFOrdEqual %v2bool %78 %111   ; RelaxedPrecision
        %119 =       OpAll %bool %118
        %120 =       OpLogicalAnd %bool %117 %119
        %121 =       OpFOrdEqual %v2bool %79 %112   ; RelaxedPrecision
        %122 =       OpAll %bool %121
        %123 =       OpLogicalAnd %bool %120 %122
        %124 =       OpFOrdEqual %v2bool %80 %113   ; RelaxedPrecision
        %125 =       OpAll %bool %124
        %126 =       OpLogicalAnd %bool %123 %125
                     OpBranch %109

        %109 = OpLabel
        %127 =   OpPhi %bool %false %26 %126 %108
                 OpSelectionMerge %129 None
                 OpBranchConditional %127 %128 %129

        %128 =     OpLabel
        %131 =       OpFOrdEqual %v3bool %92 %92
        %132 =       OpAll %bool %131
        %133 =       OpFOrdEqual %v3bool %93 %93
        %134 =       OpAll %bool %133
        %135 =       OpLogicalAnd %bool %132 %134
        %136 =       OpFOrdEqual %v3bool %94 %94
        %137 =       OpAll %bool %136
        %138 =       OpLogicalAnd %bool %135 %137
        %139 =       OpFOrdEqual %v3bool %95 %95
        %140 =       OpAll %bool %139
        %141 =       OpLogicalAnd %bool %138 %140
                     OpBranch %129

        %129 = OpLabel
        %142 =   OpPhi %bool %false %109 %141 %128
                 OpSelectionMerge %147 None
                 OpBranchConditional %142 %145 %146

        %145 =     OpLabel
        %148 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %149 =       OpLoad %v4float %148           ; RelaxedPrecision
                     OpStore %143 %149
                     OpBranch %147

        %146 =     OpLabel
        %150 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %151 =       OpLoad %v4float %150           ; RelaxedPrecision
                     OpStore %143 %151
                     OpBranch %147

        %147 = OpLabel
        %152 =   OpLoad %v4float %143               ; RelaxedPrecision
                 OpReturnValue %152
               OpFunctionEnd
