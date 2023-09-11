               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpMemberName %_UniformBuffer 2 "testMatrix2x2"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %f4 "f4"
               OpName %ok "ok"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
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
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %183 RelaxedPrecision
               OpDecorate %186 RelaxedPrecision
               OpDecorate %187 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
    %v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %17 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %24 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
        %int = OpTypeInt 32 1
      %int_2 = OpConstant %int 2
%_ptr_Function_bool = OpTypePointer Function %bool
    %v3float = OpTypeVector %float 3
%mat2v3float = OpTypeMatrix %v3float 2
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
         %53 = OpConstantComposite %v3float %float_1 %float_2 %float_3
         %54 = OpConstantComposite %v3float %float_4 %float_1 %float_2
         %55 = OpConstantComposite %mat2v3float %53 %54
     %v3bool = OpTypeVector %bool 3
      %false = OpConstantFalse %bool
%mat2v4float = OpTypeMatrix %v4float 2
         %78 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
         %79 = OpConstantComposite %mat2v4float %78 %78
     %v4bool = OpTypeVector %bool 4
%mat3v3float = OpTypeMatrix %v3float 3
        %100 = OpConstantComposite %v3float %float_3 %float_4 %float_1
        %101 = OpConstantComposite %mat3v3float %53 %54 %100
%mat4v2float = OpTypeMatrix %v2float 4
        %128 = OpConstantComposite %v2float %float_1 %float_2
        %129 = OpConstantComposite %v2float %float_3 %float_4
        %130 = OpConstantComposite %mat4v2float %128 %129 %128 %129
     %v2bool = OpTypeVector %bool 2
%mat4v3float = OpTypeMatrix %v3float 4
        %162 = OpConstantComposite %v3float %float_2 %float_3 %float_4
        %163 = OpConstantComposite %mat4v3float %53 %54 %100 %162
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %17
         %18 = OpLabel
         %21 = OpVariable %_ptr_Function_v2float Function
               OpStore %21 %20
         %23 = OpFunctionCall %v4float %main %21
               OpStore %sk_FragColor %23
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %24
         %25 = OpFunctionParameter %_ptr_Function_v2float
         %26 = OpLabel
         %f4 = OpVariable %_ptr_Function_v4float Function
         %ok = OpVariable %_ptr_Function_bool Function
        %176 = OpVariable %_ptr_Function_v4float Function
         %29 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
         %33 = OpLoad %mat2v2float %29
         %34 = OpCompositeExtract %float %33 0 0
         %35 = OpCompositeExtract %float %33 0 1
         %36 = OpCompositeExtract %float %33 1 0
         %37 = OpCompositeExtract %float %33 1 1
         %38 = OpCompositeConstruct %v4float %34 %35 %36 %37
               OpStore %f4 %38
         %41 = OpVectorShuffle %v2float %38 %38 0 1
         %43 = OpCompositeConstruct %v3float %34 %35 %36
         %44 = OpCompositeExtract %float %41 0
         %45 = OpCompositeExtract %float %41 1
         %46 = OpCompositeConstruct %v3float %37 %44 %45
         %48 = OpCompositeConstruct %mat2v3float %43 %46
         %57 = OpFOrdEqual %v3bool %43 %53
         %58 = OpAll %bool %57
         %59 = OpFOrdEqual %v3bool %46 %54
         %60 = OpAll %bool %59
         %61 = OpLogicalAnd %bool %58 %60
               OpStore %ok %61
               OpSelectionMerge %64 None
               OpBranchConditional %61 %63 %64
         %63 = OpLabel
         %65 = OpVectorShuffle %v3float %38 %38 0 1 2
         %66 = OpVectorShuffle %v4float %38 %38 3 0 1 2
         %67 = OpCompositeExtract %float %65 0
         %68 = OpCompositeExtract %float %65 1
         %69 = OpCompositeExtract %float %65 2
         %70 = OpCompositeExtract %float %66 0
         %71 = OpCompositeConstruct %v4float %67 %68 %69 %70
         %72 = OpCompositeExtract %float %66 1
         %73 = OpCompositeExtract %float %66 2
         %74 = OpCompositeExtract %float %66 3
         %75 = OpCompositeConstruct %v4float %72 %73 %74 %37
         %77 = OpCompositeConstruct %mat2v4float %71 %75
         %81 = OpFOrdEqual %v4bool %71 %78
         %82 = OpAll %bool %81
         %83 = OpFOrdEqual %v4bool %75 %78
         %84 = OpAll %bool %83
         %85 = OpLogicalAnd %bool %82 %84
               OpBranch %64
         %64 = OpLabel
         %86 = OpPhi %bool %false %26 %85 %63
               OpStore %ok %86
               OpSelectionMerge %88 None
               OpBranchConditional %86 %87 %88
         %87 = OpLabel
         %89 = OpVectorShuffle %v2float %38 %38 0 1
         %90 = OpVectorShuffle %v2float %38 %38 2 3
         %91 = OpCompositeExtract %float %89 0
         %92 = OpCompositeExtract %float %89 1
         %93 = OpCompositeExtract %float %90 0
         %94 = OpCompositeConstruct %v3float %91 %92 %93
         %95 = OpCompositeExtract %float %90 1
         %96 = OpCompositeConstruct %v3float %95 %34 %35
         %97 = OpCompositeConstruct %v3float %36 %37 %34
         %99 = OpCompositeConstruct %mat3v3float %94 %96 %97
        %102 = OpFOrdEqual %v3bool %94 %53
        %103 = OpAll %bool %102
        %104 = OpFOrdEqual %v3bool %96 %54
        %105 = OpAll %bool %104
        %106 = OpLogicalAnd %bool %103 %105
        %107 = OpFOrdEqual %v3bool %97 %100
        %108 = OpAll %bool %107
        %109 = OpLogicalAnd %bool %106 %108
               OpBranch %88
         %88 = OpLabel
        %110 = OpPhi %bool %false %64 %109 %87
               OpStore %ok %110
               OpSelectionMerge %112 None
               OpBranchConditional %110 %111 %112
        %111 = OpLabel
        %113 = OpVectorShuffle %v3float %38 %38 0 1 2
        %114 = OpVectorShuffle %v4float %38 %38 3 0 1 2
        %115 = OpCompositeExtract %float %113 0
        %116 = OpCompositeExtract %float %113 1
        %117 = OpCompositeConstruct %v2float %115 %116
        %118 = OpCompositeExtract %float %113 2
        %119 = OpCompositeExtract %float %114 0
        %120 = OpCompositeConstruct %v2float %118 %119
        %121 = OpCompositeExtract %float %114 1
        %122 = OpCompositeExtract %float %114 2
        %123 = OpCompositeConstruct %v2float %121 %122
        %124 = OpCompositeExtract %float %114 3
        %125 = OpCompositeConstruct %v2float %124 %37
        %127 = OpCompositeConstruct %mat4v2float %117 %120 %123 %125
        %132 = OpFOrdEqual %v2bool %117 %128
        %133 = OpAll %bool %132
        %134 = OpFOrdEqual %v2bool %120 %129
        %135 = OpAll %bool %134
        %136 = OpLogicalAnd %bool %133 %135
        %137 = OpFOrdEqual %v2bool %123 %128
        %138 = OpAll %bool %137
        %139 = OpLogicalAnd %bool %136 %138
        %140 = OpFOrdEqual %v2bool %125 %129
        %141 = OpAll %bool %140
        %142 = OpLogicalAnd %bool %139 %141
               OpBranch %112
        %112 = OpLabel
        %143 = OpPhi %bool %false %88 %142 %111
               OpStore %ok %143
               OpSelectionMerge %145 None
               OpBranchConditional %143 %144 %145
        %144 = OpLabel
        %146 = OpVectorShuffle %v4float %38 %38 1 2 3 0
        %147 = OpVectorShuffle %v4float %38 %38 1 2 3 0
        %148 = OpVectorShuffle %v3float %38 %38 1 2 3
        %149 = OpCompositeExtract %float %146 0
        %150 = OpCompositeExtract %float %146 1
        %151 = OpCompositeConstruct %v3float %34 %149 %150
        %152 = OpCompositeExtract %float %146 2
        %153 = OpCompositeExtract %float %146 3
        %154 = OpCompositeExtract %float %147 0
        %155 = OpCompositeConstruct %v3float %152 %153 %154
        %156 = OpCompositeExtract %float %147 1
        %157 = OpCompositeExtract %float %147 2
        %158 = OpCompositeExtract %float %147 3
        %159 = OpCompositeConstruct %v3float %156 %157 %158
        %161 = OpCompositeConstruct %mat4v3float %151 %155 %159 %148
        %164 = OpFOrdEqual %v3bool %151 %53
        %165 = OpAll %bool %164
        %166 = OpFOrdEqual %v3bool %155 %54
        %167 = OpAll %bool %166
        %168 = OpLogicalAnd %bool %165 %167
        %169 = OpFOrdEqual %v3bool %159 %100
        %170 = OpAll %bool %169
        %171 = OpLogicalAnd %bool %168 %170
        %172 = OpFOrdEqual %v3bool %148 %162
        %173 = OpAll %bool %172
        %174 = OpLogicalAnd %bool %171 %173
               OpBranch %145
        %145 = OpLabel
        %175 = OpPhi %bool %false %112 %174 %144
               OpStore %ok %175
               OpSelectionMerge %179 None
               OpBranchConditional %175 %177 %178
        %177 = OpLabel
        %180 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %183 = OpLoad %v4float %180
               OpStore %176 %183
               OpBranch %179
        %178 = OpLabel
        %184 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %186 = OpLoad %v4float %184
               OpStore %176 %186
               OpBranch %179
        %179 = OpLabel
        %187 = OpLoad %v4float %176
               OpReturnValue %187
               OpFunctionEnd
