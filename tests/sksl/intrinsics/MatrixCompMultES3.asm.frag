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
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %h24 "h24"
               OpName %h42 "h42"
               OpName %f43 "f43"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %h24 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %h42 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
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
               OpDecorate %82 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %125 RelaxedPrecision
               OpDecorate %150 RelaxedPrecision
               OpDecorate %152 RelaxedPrecision
               OpDecorate %153 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %23 = OpTypeFunction %v4float %_ptr_Function_v2float
%mat2v4float = OpTypeMatrix %v4float 2
%_ptr_Function_mat2v4float = OpTypePointer Function %mat2v4float
    %float_9 = OpConstant %float 9
         %31 = OpConstantComposite %v4float %float_9 %float_9 %float_9 %float_9
         %32 = OpConstantComposite %mat2v4float %31 %31
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
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
         %57 = OpConstantComposite %v2float %float_1 %float_2
         %58 = OpConstantComposite %v2float %float_3 %float_4
         %59 = OpConstantComposite %v2float %float_5 %float_6
         %60 = OpConstantComposite %v2float %float_7 %float_8
         %61 = OpConstantComposite %mat4v2float %57 %58 %59 %60
    %v3float = OpTypeVector %float 3
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Function_mat4v3float = OpTypePointer Function %mat4v3float
   %float_12 = OpConstant %float 12
   %float_22 = OpConstant %float 22
   %float_30 = OpConstant %float 30
   %float_36 = OpConstant %float 36
   %float_40 = OpConstant %float 40
   %float_42 = OpConstant %float 42
         %94 = OpConstantComposite %v3float %float_12 %float_22 %float_30
         %95 = OpConstantComposite %v3float %float_36 %float_40 %float_42
         %96 = OpConstantComposite %v3float %float_42 %float_40 %float_36
         %97 = OpConstantComposite %v3float %float_30 %float_22 %float_12
         %98 = OpConstantComposite %mat4v3float %94 %95 %96 %97
      %false = OpConstantFalse %bool
        %100 = OpConstantComposite %v4float %float_9 %float_0 %float_0 %float_9
        %101 = OpConstantComposite %v4float %float_0 %float_9 %float_0 %float_9
        %102 = OpConstantComposite %mat2v4float %100 %101
     %v4bool = OpTypeVector %bool 4
        %111 = OpConstantComposite %v2float %float_1 %float_0
        %112 = OpConstantComposite %v2float %float_0 %float_4
        %113 = OpConstantComposite %v2float %float_0 %float_6
        %114 = OpConstantComposite %v2float %float_0 %float_8
        %115 = OpConstantComposite %mat4v2float %111 %112 %113 %114
     %v2bool = OpTypeVector %bool 2
     %v3bool = OpTypeVector %bool 3
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %15
         %16 = OpLabel
         %20 = OpVariable %_ptr_Function_v2float Function
               OpStore %20 %19
         %22 = OpFunctionCall %v4float %main %20
               OpStore %sk_FragColor %22
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %23
         %24 = OpFunctionParameter %_ptr_Function_v2float
         %25 = OpLabel
        %h24 = OpVariable %_ptr_Function_mat2v4float Function
        %h42 = OpVariable %_ptr_Function_mat4v2float Function
        %f43 = OpVariable %_ptr_Function_mat4v3float Function
        %144 = OpVariable %_ptr_Function_v4float Function
         %33 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %37 = OpLoad %v4float %33
         %38 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %40 = OpLoad %v4float %38
         %41 = OpCompositeConstruct %mat2v4float %37 %40
         %42 = OpFMul %v4float %31 %37
         %43 = OpFMul %v4float %31 %40
         %44 = OpCompositeConstruct %mat2v4float %42 %43
               OpStore %h24 %44
         %62 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %63 = OpLoad %v4float %62
         %64 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %65 = OpLoad %v4float %64
         %66 = OpCompositeExtract %float %63 0
         %67 = OpCompositeExtract %float %63 1
         %68 = OpCompositeConstruct %v2float %66 %67
         %69 = OpCompositeExtract %float %63 2
         %70 = OpCompositeExtract %float %63 3
         %71 = OpCompositeConstruct %v2float %69 %70
         %72 = OpCompositeExtract %float %65 0
         %73 = OpCompositeExtract %float %65 1
         %74 = OpCompositeConstruct %v2float %72 %73
         %75 = OpCompositeExtract %float %65 2
         %76 = OpCompositeExtract %float %65 3
         %77 = OpCompositeConstruct %v2float %75 %76
         %78 = OpCompositeConstruct %mat4v2float %68 %71 %74 %77
         %79 = OpFMul %v2float %57 %68
         %80 = OpFMul %v2float %58 %71
         %81 = OpFMul %v2float %59 %74
         %82 = OpFMul %v2float %60 %77
         %83 = OpCompositeConstruct %mat4v2float %79 %80 %81 %82
               OpStore %h42 %83
               OpStore %f43 %98
        %104 = OpFOrdEqual %v4bool %42 %100
        %105 = OpAll %bool %104
        %106 = OpFOrdEqual %v4bool %43 %101
        %107 = OpAll %bool %106
        %108 = OpLogicalAnd %bool %105 %107
               OpSelectionMerge %110 None
               OpBranchConditional %108 %109 %110
        %109 = OpLabel
        %117 = OpFOrdEqual %v2bool %79 %111
        %118 = OpAll %bool %117
        %119 = OpFOrdEqual %v2bool %80 %112
        %120 = OpAll %bool %119
        %121 = OpLogicalAnd %bool %118 %120
        %122 = OpFOrdEqual %v2bool %81 %113
        %123 = OpAll %bool %122
        %124 = OpLogicalAnd %bool %121 %123
        %125 = OpFOrdEqual %v2bool %82 %114
        %126 = OpAll %bool %125
        %127 = OpLogicalAnd %bool %124 %126
               OpBranch %110
        %110 = OpLabel
        %128 = OpPhi %bool %false %25 %127 %109
               OpSelectionMerge %130 None
               OpBranchConditional %128 %129 %130
        %129 = OpLabel
        %132 = OpFOrdEqual %v3bool %94 %94
        %133 = OpAll %bool %132
        %134 = OpFOrdEqual %v3bool %95 %95
        %135 = OpAll %bool %134
        %136 = OpLogicalAnd %bool %133 %135
        %137 = OpFOrdEqual %v3bool %96 %96
        %138 = OpAll %bool %137
        %139 = OpLogicalAnd %bool %136 %138
        %140 = OpFOrdEqual %v3bool %97 %97
        %141 = OpAll %bool %140
        %142 = OpLogicalAnd %bool %139 %141
               OpBranch %130
        %130 = OpLabel
        %143 = OpPhi %bool %false %110 %142 %129
               OpSelectionMerge %148 None
               OpBranchConditional %143 %146 %147
        %146 = OpLabel
        %149 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %150 = OpLoad %v4float %149
               OpStore %144 %150
               OpBranch %148
        %147 = OpLabel
        %151 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %152 = OpLoad %v4float %151
               OpStore %144 %152
               OpBranch %148
        %148 = OpLabel
        %153 = OpLoad %v4float %144
               OpReturnValue %153
               OpFunctionEnd
