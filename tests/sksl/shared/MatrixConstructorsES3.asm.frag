               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %_UniformBuffer "_UniformBuffer"  ; id %14
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpMemberName %_UniformBuffer 2 "testMatrix2x2"
               OpName %_entrypoint_v "_entrypoint_v"    ; id %16
               OpName %main "main"                      ; id %6
               OpName %f4 "f4"                          ; id %28
               OpName %ok "ok"                          ; id %39

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpMemberDecorate %_UniformBuffer 2 ColMajor
               OpMemberDecorate %_UniformBuffer 2 MatrixStride 16
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %184 RelaxedPrecision
               OpDecorate %187 RelaxedPrecision
               OpDecorate %188 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
    %v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float   ; Block
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform  ; Binding 0, DescriptorSet 0
       %void = OpTypeVoid
         %18 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %25 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
      %int_2 = OpConstant %int 2
       %bool = OpTypeBool
%_ptr_Function_bool = OpTypePointer Function %bool
    %v3float = OpTypeVector %float 3
%mat2v3float = OpTypeMatrix %v3float 2
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
         %54 = OpConstantComposite %v3float %float_1 %float_2 %float_3
         %55 = OpConstantComposite %v3float %float_4 %float_1 %float_2
         %56 = OpConstantComposite %mat2v3float %54 %55
     %v3bool = OpTypeVector %bool 3
      %false = OpConstantFalse %bool
%mat2v4float = OpTypeMatrix %v4float 2
         %79 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
         %80 = OpConstantComposite %mat2v4float %79 %79
     %v4bool = OpTypeVector %bool 4
%mat3v3float = OpTypeMatrix %v3float 3
        %101 = OpConstantComposite %v3float %float_3 %float_4 %float_1
        %102 = OpConstantComposite %mat3v3float %54 %55 %101
%mat4v2float = OpTypeMatrix %v2float 4
        %129 = OpConstantComposite %v2float %float_1 %float_2
        %130 = OpConstantComposite %v2float %float_3 %float_4
        %131 = OpConstantComposite %mat4v2float %129 %130 %129 %130
     %v2bool = OpTypeVector %bool 2
%mat4v3float = OpTypeMatrix %v3float 4
        %163 = OpConstantComposite %v3float %float_2 %float_3 %float_4
        %164 = OpConstantComposite %mat4v3float %54 %55 %101 %163
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %18

         %19 = OpLabel
         %22 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %22 %21
         %24 =   OpFunctionCall %v4float %main %22
                 OpStore %sk_FragColor %24
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %25         ; RelaxedPrecision
         %26 = OpFunctionParameter %_ptr_Function_v2float

         %27 = OpLabel
         %f4 =   OpVariable %_ptr_Function_v4float Function
         %ok =   OpVariable %_ptr_Function_bool Function
        %177 =   OpVariable %_ptr_Function_v4float Function
         %30 =   OpAccessChain %_ptr_Uniform_mat2v2float %11 %int_2
         %33 =   OpLoad %mat2v2float %30
         %34 =   OpCompositeExtract %float %33 0 0
         %35 =   OpCompositeExtract %float %33 0 1
         %36 =   OpCompositeExtract %float %33 1 0
         %37 =   OpCompositeExtract %float %33 1 1
         %38 =   OpCompositeConstruct %v4float %34 %35 %36 %37
                 OpStore %f4 %38
         %42 =   OpVectorShuffle %v2float %38 %38 0 1
         %44 =   OpCompositeConstruct %v3float %34 %35 %36
         %45 =   OpCompositeExtract %float %42 0
         %46 =   OpCompositeExtract %float %42 1
         %47 =   OpCompositeConstruct %v3float %37 %45 %46
         %49 =   OpCompositeConstruct %mat2v3float %44 %47
         %58 =   OpFOrdEqual %v3bool %44 %54
         %59 =   OpAll %bool %58
         %60 =   OpFOrdEqual %v3bool %47 %55
         %61 =   OpAll %bool %60
         %62 =   OpLogicalAnd %bool %59 %61
                 OpStore %ok %62
                 OpSelectionMerge %65 None
                 OpBranchConditional %62 %64 %65

         %64 =     OpLabel
         %66 =       OpVectorShuffle %v3float %38 %38 0 1 2
         %67 =       OpVectorShuffle %v4float %38 %38 3 0 1 2
         %68 =       OpCompositeExtract %float %66 0
         %69 =       OpCompositeExtract %float %66 1
         %70 =       OpCompositeExtract %float %66 2
         %71 =       OpCompositeExtract %float %67 0
         %72 =       OpCompositeConstruct %v4float %68 %69 %70 %71
         %73 =       OpCompositeExtract %float %67 1
         %74 =       OpCompositeExtract %float %67 2
         %75 =       OpCompositeExtract %float %67 3
         %76 =       OpCompositeConstruct %v4float %73 %74 %75 %37
         %78 =       OpCompositeConstruct %mat2v4float %72 %76
         %82 =       OpFOrdEqual %v4bool %72 %79
         %83 =       OpAll %bool %82
         %84 =       OpFOrdEqual %v4bool %76 %79
         %85 =       OpAll %bool %84
         %86 =       OpLogicalAnd %bool %83 %85
                     OpBranch %65

         %65 = OpLabel
         %87 =   OpPhi %bool %false %27 %86 %64
                 OpStore %ok %87
                 OpSelectionMerge %89 None
                 OpBranchConditional %87 %88 %89

         %88 =     OpLabel
         %90 =       OpVectorShuffle %v2float %38 %38 0 1
         %91 =       OpVectorShuffle %v2float %38 %38 2 3
         %92 =       OpCompositeExtract %float %90 0
         %93 =       OpCompositeExtract %float %90 1
         %94 =       OpCompositeExtract %float %91 0
         %95 =       OpCompositeConstruct %v3float %92 %93 %94
         %96 =       OpCompositeExtract %float %91 1
         %97 =       OpCompositeConstruct %v3float %96 %34 %35
         %98 =       OpCompositeConstruct %v3float %36 %37 %34
        %100 =       OpCompositeConstruct %mat3v3float %95 %97 %98
        %103 =       OpFOrdEqual %v3bool %95 %54
        %104 =       OpAll %bool %103
        %105 =       OpFOrdEqual %v3bool %97 %55
        %106 =       OpAll %bool %105
        %107 =       OpLogicalAnd %bool %104 %106
        %108 =       OpFOrdEqual %v3bool %98 %101
        %109 =       OpAll %bool %108
        %110 =       OpLogicalAnd %bool %107 %109
                     OpBranch %89

         %89 = OpLabel
        %111 =   OpPhi %bool %false %65 %110 %88
                 OpStore %ok %111
                 OpSelectionMerge %113 None
                 OpBranchConditional %111 %112 %113

        %112 =     OpLabel
        %114 =       OpVectorShuffle %v3float %38 %38 0 1 2
        %115 =       OpVectorShuffle %v4float %38 %38 3 0 1 2
        %116 =       OpCompositeExtract %float %114 0
        %117 =       OpCompositeExtract %float %114 1
        %118 =       OpCompositeConstruct %v2float %116 %117
        %119 =       OpCompositeExtract %float %114 2
        %120 =       OpCompositeExtract %float %115 0
        %121 =       OpCompositeConstruct %v2float %119 %120
        %122 =       OpCompositeExtract %float %115 1
        %123 =       OpCompositeExtract %float %115 2
        %124 =       OpCompositeConstruct %v2float %122 %123
        %125 =       OpCompositeExtract %float %115 3
        %126 =       OpCompositeConstruct %v2float %125 %37
        %128 =       OpCompositeConstruct %mat4v2float %118 %121 %124 %126
        %133 =       OpFOrdEqual %v2bool %118 %129
        %134 =       OpAll %bool %133
        %135 =       OpFOrdEqual %v2bool %121 %130
        %136 =       OpAll %bool %135
        %137 =       OpLogicalAnd %bool %134 %136
        %138 =       OpFOrdEqual %v2bool %124 %129
        %139 =       OpAll %bool %138
        %140 =       OpLogicalAnd %bool %137 %139
        %141 =       OpFOrdEqual %v2bool %126 %130
        %142 =       OpAll %bool %141
        %143 =       OpLogicalAnd %bool %140 %142
                     OpBranch %113

        %113 = OpLabel
        %144 =   OpPhi %bool %false %89 %143 %112
                 OpStore %ok %144
                 OpSelectionMerge %146 None
                 OpBranchConditional %144 %145 %146

        %145 =     OpLabel
        %147 =       OpVectorShuffle %v4float %38 %38 1 2 3 0
        %148 =       OpVectorShuffle %v4float %38 %38 1 2 3 0
        %149 =       OpVectorShuffle %v3float %38 %38 1 2 3
        %150 =       OpCompositeExtract %float %147 0
        %151 =       OpCompositeExtract %float %147 1
        %152 =       OpCompositeConstruct %v3float %34 %150 %151
        %153 =       OpCompositeExtract %float %147 2
        %154 =       OpCompositeExtract %float %147 3
        %155 =       OpCompositeExtract %float %148 0
        %156 =       OpCompositeConstruct %v3float %153 %154 %155
        %157 =       OpCompositeExtract %float %148 1
        %158 =       OpCompositeExtract %float %148 2
        %159 =       OpCompositeExtract %float %148 3
        %160 =       OpCompositeConstruct %v3float %157 %158 %159
        %162 =       OpCompositeConstruct %mat4v3float %152 %156 %160 %149
        %165 =       OpFOrdEqual %v3bool %152 %54
        %166 =       OpAll %bool %165
        %167 =       OpFOrdEqual %v3bool %156 %55
        %168 =       OpAll %bool %167
        %169 =       OpLogicalAnd %bool %166 %168
        %170 =       OpFOrdEqual %v3bool %160 %101
        %171 =       OpAll %bool %170
        %172 =       OpLogicalAnd %bool %169 %171
        %173 =       OpFOrdEqual %v3bool %149 %163
        %174 =       OpAll %bool %173
        %175 =       OpLogicalAnd %bool %172 %174
                     OpBranch %146

        %146 = OpLabel
        %176 =   OpPhi %bool %false %113 %175 %145
                 OpStore %ok %176
                 OpSelectionMerge %180 None
                 OpBranchConditional %176 %178 %179

        %178 =     OpLabel
        %181 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %184 =       OpLoad %v4float %181           ; RelaxedPrecision
                     OpStore %177 %184
                     OpBranch %180

        %179 =     OpLabel
        %185 =       OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %187 =       OpLoad %v4float %185           ; RelaxedPrecision
                     OpStore %177 %187
                     OpBranch %180

        %180 = OpLabel
        %188 =   OpLoad %v4float %177               ; RelaxedPrecision
                 OpReturnValue %188
               OpFunctionEnd
