               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpMemberName %_UniformBuffer 2 "testMatrix2x2"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %f4 "f4"
               OpName %ok "ok"
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
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %181 RelaxedPrecision
               OpDecorate %184 RelaxedPrecision
               OpDecorate %185 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
    %v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %14 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %17 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %21 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
        %int = OpTypeInt 32 1
      %int_2 = OpConstant %int 2
       %bool = OpTypeBool
%_ptr_Function_bool = OpTypePointer Function %bool
    %v3float = OpTypeVector %float 3
%mat2v3float = OpTypeMatrix %v3float 2
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
         %51 = OpConstantComposite %v3float %float_1 %float_2 %float_3
         %52 = OpConstantComposite %v3float %float_4 %float_1 %float_2
         %53 = OpConstantComposite %mat2v3float %51 %52
     %v3bool = OpTypeVector %bool 3
      %false = OpConstantFalse %bool
%mat2v4float = OpTypeMatrix %v4float 2
         %76 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
         %77 = OpConstantComposite %mat2v4float %76 %76
     %v4bool = OpTypeVector %bool 4
%mat3v3float = OpTypeMatrix %v3float 3
         %98 = OpConstantComposite %v3float %float_3 %float_4 %float_1
         %99 = OpConstantComposite %mat3v3float %51 %52 %98
%mat4v2float = OpTypeMatrix %v2float 4
        %126 = OpConstantComposite %v2float %float_1 %float_2
        %127 = OpConstantComposite %v2float %float_3 %float_4
        %128 = OpConstantComposite %mat4v2float %126 %127 %126 %127
     %v2bool = OpTypeVector %bool 2
%mat4v3float = OpTypeMatrix %v3float 4
        %160 = OpConstantComposite %v3float %float_2 %float_3 %float_4
        %161 = OpConstantComposite %mat4v3float %51 %52 %98 %160
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %14
         %15 = OpLabel
         %18 = OpVariable %_ptr_Function_v2float Function
               OpStore %18 %17
         %20 = OpFunctionCall %v4float %main %18
               OpStore %sk_FragColor %20
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %21
         %22 = OpFunctionParameter %_ptr_Function_v2float
         %23 = OpLabel
         %f4 = OpVariable %_ptr_Function_v4float Function
         %ok = OpVariable %_ptr_Function_bool Function
        %174 = OpVariable %_ptr_Function_v4float Function
         %26 = OpAccessChain %_ptr_Uniform_mat2v2float %7 %int_2
         %30 = OpLoad %mat2v2float %26
         %31 = OpCompositeExtract %float %30 0 0
         %32 = OpCompositeExtract %float %30 0 1
         %33 = OpCompositeExtract %float %30 1 0
         %34 = OpCompositeExtract %float %30 1 1
         %35 = OpCompositeConstruct %v4float %31 %32 %33 %34
               OpStore %f4 %35
         %39 = OpVectorShuffle %v2float %35 %35 0 1
         %41 = OpCompositeConstruct %v3float %31 %32 %33
         %42 = OpCompositeExtract %float %39 0
         %43 = OpCompositeExtract %float %39 1
         %44 = OpCompositeConstruct %v3float %34 %42 %43
         %46 = OpCompositeConstruct %mat2v3float %41 %44
         %55 = OpFOrdEqual %v3bool %41 %51
         %56 = OpAll %bool %55
         %57 = OpFOrdEqual %v3bool %44 %52
         %58 = OpAll %bool %57
         %59 = OpLogicalAnd %bool %56 %58
               OpStore %ok %59
               OpSelectionMerge %62 None
               OpBranchConditional %59 %61 %62
         %61 = OpLabel
         %63 = OpVectorShuffle %v3float %35 %35 0 1 2
         %64 = OpVectorShuffle %v4float %35 %35 3 0 1 2
         %65 = OpCompositeExtract %float %63 0
         %66 = OpCompositeExtract %float %63 1
         %67 = OpCompositeExtract %float %63 2
         %68 = OpCompositeExtract %float %64 0
         %69 = OpCompositeConstruct %v4float %65 %66 %67 %68
         %70 = OpCompositeExtract %float %64 1
         %71 = OpCompositeExtract %float %64 2
         %72 = OpCompositeExtract %float %64 3
         %73 = OpCompositeConstruct %v4float %70 %71 %72 %34
         %75 = OpCompositeConstruct %mat2v4float %69 %73
         %79 = OpFOrdEqual %v4bool %69 %76
         %80 = OpAll %bool %79
         %81 = OpFOrdEqual %v4bool %73 %76
         %82 = OpAll %bool %81
         %83 = OpLogicalAnd %bool %80 %82
               OpBranch %62
         %62 = OpLabel
         %84 = OpPhi %bool %false %23 %83 %61
               OpStore %ok %84
               OpSelectionMerge %86 None
               OpBranchConditional %84 %85 %86
         %85 = OpLabel
         %87 = OpVectorShuffle %v2float %35 %35 0 1
         %88 = OpVectorShuffle %v2float %35 %35 2 3
         %89 = OpCompositeExtract %float %87 0
         %90 = OpCompositeExtract %float %87 1
         %91 = OpCompositeExtract %float %88 0
         %92 = OpCompositeConstruct %v3float %89 %90 %91
         %93 = OpCompositeExtract %float %88 1
         %94 = OpCompositeConstruct %v3float %93 %31 %32
         %95 = OpCompositeConstruct %v3float %33 %34 %31
         %97 = OpCompositeConstruct %mat3v3float %92 %94 %95
        %100 = OpFOrdEqual %v3bool %92 %51
        %101 = OpAll %bool %100
        %102 = OpFOrdEqual %v3bool %94 %52
        %103 = OpAll %bool %102
        %104 = OpLogicalAnd %bool %101 %103
        %105 = OpFOrdEqual %v3bool %95 %98
        %106 = OpAll %bool %105
        %107 = OpLogicalAnd %bool %104 %106
               OpBranch %86
         %86 = OpLabel
        %108 = OpPhi %bool %false %62 %107 %85
               OpStore %ok %108
               OpSelectionMerge %110 None
               OpBranchConditional %108 %109 %110
        %109 = OpLabel
        %111 = OpVectorShuffle %v3float %35 %35 0 1 2
        %112 = OpVectorShuffle %v4float %35 %35 3 0 1 2
        %113 = OpCompositeExtract %float %111 0
        %114 = OpCompositeExtract %float %111 1
        %115 = OpCompositeConstruct %v2float %113 %114
        %116 = OpCompositeExtract %float %111 2
        %117 = OpCompositeExtract %float %112 0
        %118 = OpCompositeConstruct %v2float %116 %117
        %119 = OpCompositeExtract %float %112 1
        %120 = OpCompositeExtract %float %112 2
        %121 = OpCompositeConstruct %v2float %119 %120
        %122 = OpCompositeExtract %float %112 3
        %123 = OpCompositeConstruct %v2float %122 %34
        %125 = OpCompositeConstruct %mat4v2float %115 %118 %121 %123
        %130 = OpFOrdEqual %v2bool %115 %126
        %131 = OpAll %bool %130
        %132 = OpFOrdEqual %v2bool %118 %127
        %133 = OpAll %bool %132
        %134 = OpLogicalAnd %bool %131 %133
        %135 = OpFOrdEqual %v2bool %121 %126
        %136 = OpAll %bool %135
        %137 = OpLogicalAnd %bool %134 %136
        %138 = OpFOrdEqual %v2bool %123 %127
        %139 = OpAll %bool %138
        %140 = OpLogicalAnd %bool %137 %139
               OpBranch %110
        %110 = OpLabel
        %141 = OpPhi %bool %false %86 %140 %109
               OpStore %ok %141
               OpSelectionMerge %143 None
               OpBranchConditional %141 %142 %143
        %142 = OpLabel
        %144 = OpVectorShuffle %v4float %35 %35 1 2 3 0
        %145 = OpVectorShuffle %v4float %35 %35 1 2 3 0
        %146 = OpVectorShuffle %v3float %35 %35 1 2 3
        %147 = OpCompositeExtract %float %144 0
        %148 = OpCompositeExtract %float %144 1
        %149 = OpCompositeConstruct %v3float %31 %147 %148
        %150 = OpCompositeExtract %float %144 2
        %151 = OpCompositeExtract %float %144 3
        %152 = OpCompositeExtract %float %145 0
        %153 = OpCompositeConstruct %v3float %150 %151 %152
        %154 = OpCompositeExtract %float %145 1
        %155 = OpCompositeExtract %float %145 2
        %156 = OpCompositeExtract %float %145 3
        %157 = OpCompositeConstruct %v3float %154 %155 %156
        %159 = OpCompositeConstruct %mat4v3float %149 %153 %157 %146
        %162 = OpFOrdEqual %v3bool %149 %51
        %163 = OpAll %bool %162
        %164 = OpFOrdEqual %v3bool %153 %52
        %165 = OpAll %bool %164
        %166 = OpLogicalAnd %bool %163 %165
        %167 = OpFOrdEqual %v3bool %157 %98
        %168 = OpAll %bool %167
        %169 = OpLogicalAnd %bool %166 %168
        %170 = OpFOrdEqual %v3bool %146 %160
        %171 = OpAll %bool %170
        %172 = OpLogicalAnd %bool %169 %171
               OpBranch %143
        %143 = OpLabel
        %173 = OpPhi %bool %false %110 %172 %142
               OpStore %ok %173
               OpSelectionMerge %177 None
               OpBranchConditional %173 %175 %176
        %175 = OpLabel
        %178 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %181 = OpLoad %v4float %178
               OpStore %174 %181
               OpBranch %177
        %176 = OpLabel
        %182 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %184 = OpLoad %v4float %182
               OpStore %174 %184
               OpBranch %177
        %177 = OpLabel
        %185 = OpLoad %v4float %174
               OpReturnValue %185
               OpFunctionEnd
