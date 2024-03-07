               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %checkIntrinsicAsFunctionArg_bf3i3 "checkIntrinsicAsFunctionArg_bf3i3"
               OpName %main "main"
               OpName %value "value"
               OpName %exp "exp"
               OpName %result "result"
               OpName %ok "ok"
               OpName %funcOk "funcOk"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %8 Binding 0
               OpDecorate %8 DescriptorSet 0
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %148 RelaxedPrecision
               OpDecorate %157 RelaxedPrecision
               OpDecorate %159 RelaxedPrecision
               OpDecorate %160 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %8 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %13 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %17 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
    %v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
        %int = OpTypeInt 32 1
      %v3int = OpTypeVector %int 3
%_ptr_Function_v3int = OpTypePointer Function %v3int
         %27 = OpTypeFunction %bool %_ptr_Function_v3float %_ptr_Function_v3int
      %false = OpConstantFalse %bool
 %float_0_75 = OpConstant %float 0.75
         %34 = OpConstantComposite %v3float %float_0_75 %float_0_75 %float_0_75
     %v3bool = OpTypeVector %bool 3
      %int_3 = OpConstant %int 3
         %42 = OpConstantComposite %v3int %int_3 %int_3 %int_3
         %46 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
    %float_6 = OpConstant %float 6
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
     %v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Function_bool = OpTypePointer Function %bool
      %v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
%_entrypoint_v = OpFunction %void None %13
         %14 = OpLabel
         %18 = OpVariable %_ptr_Function_v2float Function
               OpStore %18 %17
         %20 = OpFunctionCall %v4float %main %18
               OpStore %sk_FragColor %20
               OpReturn
               OpFunctionEnd
%checkIntrinsicAsFunctionArg_bf3i3 = OpFunction %bool None %27
         %28 = OpFunctionParameter %_ptr_Function_v3float
         %29 = OpFunctionParameter %_ptr_Function_v3int
         %30 = OpLabel
         %32 = OpLoad %v3float %28
         %35 = OpFOrdEqual %v3bool %32 %34
         %37 = OpAll %bool %35
               OpSelectionMerge %39 None
               OpBranchConditional %37 %38 %39
         %38 = OpLabel
         %40 = OpLoad %v3int %29
         %43 = OpIEqual %v3bool %40 %42
         %44 = OpAll %bool %43
               OpBranch %39
         %39 = OpLabel
         %45 = OpPhi %bool %false %30 %44 %38
               OpReturnValue %45
               OpFunctionEnd
       %main = OpFunction %v4float None %46
         %47 = OpFunctionParameter %_ptr_Function_v2float
         %48 = OpLabel
      %value = OpVariable %_ptr_Function_v4float Function
        %exp = OpVariable %_ptr_Function_v4int Function
     %result = OpVariable %_ptr_Function_v4float Function
         %ok = OpVariable %_ptr_Function_v4bool Function
         %69 = OpVariable %_ptr_Function_int Function
         %87 = OpVariable %_ptr_Function_v2int Function
        %107 = OpVariable %_ptr_Function_v3int Function
        %124 = OpVariable %_ptr_Function_v4int Function
     %funcOk = OpVariable %_ptr_Function_bool Function
        %138 = OpVariable %_ptr_Function_v3int Function
        %143 = OpVariable %_ptr_Function_v3float Function
        %145 = OpVariable %_ptr_Function_v3int Function
        %152 = OpVariable %_ptr_Function_v4float Function
         %51 = OpAccessChain %_ptr_Uniform_v4float %8 %int_0
         %54 = OpLoad %v4float %51
         %55 = OpVectorShuffle %v4float %54 %54 1 1 1 1
         %57 = OpVectorTimesScalar %v4float %55 %float_6
               OpStore %value %57
         %66 = OpCompositeExtract %float %57 0
         %67 = OpAccessChain %_ptr_Function_int %exp %int_0
         %65 = OpExtInst %float %1 Frexp %66 %69
         %70 = OpLoad %int %69
               OpStore %67 %70
         %71 = OpAccessChain %_ptr_Function_float %result %int_0
               OpStore %71 %65
         %73 = OpLoad %v4float %result
         %74 = OpCompositeExtract %float %73 0
         %75 = OpFOrdEqual %bool %74 %float_0_75
               OpSelectionMerge %77 None
               OpBranchConditional %75 %76 %77
         %76 = OpLabel
         %78 = OpLoad %v4int %exp
         %79 = OpCompositeExtract %int %78 0
         %80 = OpIEqual %bool %79 %int_3
               OpBranch %77
         %77 = OpLabel
         %81 = OpPhi %bool %false %48 %80 %76
         %82 = OpAccessChain %_ptr_Function_bool %ok %int_0
               OpStore %82 %81
         %85 = OpLoad %v4float %value
         %86 = OpVectorShuffle %v2float %85 %85 0 1
         %84 = OpExtInst %v2float %1 Frexp %86 %87
         %90 = OpLoad %v2int %87
         %91 = OpLoad %v4int %exp
         %92 = OpVectorShuffle %v4int %91 %90 4 5 2 3
               OpStore %exp %92
         %93 = OpLoad %v4float %result
         %94 = OpVectorShuffle %v4float %93 %84 4 5 2 3
               OpStore %result %94
         %95 = OpCompositeExtract %float %94 1
         %96 = OpFOrdEqual %bool %95 %float_0_75
               OpSelectionMerge %98 None
               OpBranchConditional %96 %97 %98
         %97 = OpLabel
         %99 = OpCompositeExtract %int %92 1
        %100 = OpIEqual %bool %99 %int_3
               OpBranch %98
         %98 = OpLabel
        %101 = OpPhi %bool %false %77 %100 %97
        %102 = OpAccessChain %_ptr_Function_bool %ok %int_1
               OpStore %102 %101
        %105 = OpLoad %v4float %value
        %106 = OpVectorShuffle %v3float %105 %105 0 1 2
        %104 = OpExtInst %v3float %1 Frexp %106 %107
        %108 = OpLoad %v3int %107
        %109 = OpLoad %v4int %exp
        %110 = OpVectorShuffle %v4int %109 %108 4 5 6 3
               OpStore %exp %110
        %111 = OpLoad %v4float %result
        %112 = OpVectorShuffle %v4float %111 %104 4 5 6 3
               OpStore %result %112
        %113 = OpCompositeExtract %float %112 2
        %114 = OpFOrdEqual %bool %113 %float_0_75
               OpSelectionMerge %116 None
               OpBranchConditional %114 %115 %116
        %115 = OpLabel
        %117 = OpCompositeExtract %int %110 2
        %118 = OpIEqual %bool %117 %int_3
               OpBranch %116
        %116 = OpLabel
        %119 = OpPhi %bool %false %98 %118 %115
        %120 = OpAccessChain %_ptr_Function_bool %ok %int_2
               OpStore %120 %119
        %123 = OpLoad %v4float %value
        %122 = OpExtInst %v4float %1 Frexp %123 %124
        %125 = OpLoad %v4int %124
               OpStore %exp %125
               OpStore %result %122
        %126 = OpCompositeExtract %float %122 3
        %127 = OpFOrdEqual %bool %126 %float_0_75
               OpSelectionMerge %129 None
               OpBranchConditional %127 %128 %129
        %128 = OpLabel
        %130 = OpCompositeExtract %int %125 3
        %131 = OpIEqual %bool %130 %int_3
               OpBranch %129
        %129 = OpLabel
        %132 = OpPhi %bool %false %116 %131 %128
        %133 = OpAccessChain %_ptr_Function_bool %ok %int_3
               OpStore %133 %132
        %136 = OpLoad %v4float %value
        %137 = OpVectorShuffle %v3float %136 %136 3 2 1
        %135 = OpExtInst %v3float %1 Frexp %137 %138
        %139 = OpLoad %v3int %138
        %140 = OpLoad %v4int %exp
        %141 = OpVectorShuffle %v4int %140 %139 5 1 4 6
               OpStore %exp %141
        %142 = OpVectorShuffle %v3float %135 %135 1 0 2
               OpStore %143 %142
        %144 = OpVectorShuffle %v3int %141 %141 1 0 2
               OpStore %145 %144
        %146 = OpFunctionCall %bool %checkIntrinsicAsFunctionArg_bf3i3 %143 %145
               OpStore %funcOk %146
        %148 = OpLoad %v4bool %ok
        %147 = OpAll %bool %148
               OpSelectionMerge %150 None
               OpBranchConditional %147 %149 %150
        %149 = OpLabel
               OpBranch %150
        %150 = OpLabel
        %151 = OpPhi %bool %false %129 %146 %149
               OpSelectionMerge %155 None
               OpBranchConditional %151 %153 %154
        %153 = OpLabel
        %156 = OpAccessChain %_ptr_Uniform_v4float %8 %int_0
        %157 = OpLoad %v4float %156
               OpStore %152 %157
               OpBranch %155
        %154 = OpLabel
        %158 = OpAccessChain %_ptr_Uniform_v4float %8 %int_1
        %159 = OpLoad %v4float %158
               OpStore %152 %159
               OpBranch %155
        %155 = OpLabel
        %160 = OpLoad %v4float %152
               OpReturnValue %160
               OpFunctionEnd
