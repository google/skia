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
               OpMemberName %_UniformBuffer 2 "unknownInput"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3i4 "check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3i4"
               OpName %main "main"
               OpName %v1 "v1"
               OpName %v2 "v2"
               OpName %v3 "v3"
               OpName %v4 "v4"
               OpName %v5 "v5"
               OpName %v6 "v6"
               OpName %v7 "v7"
               OpName %v8 "v8"
               OpName %v9 "v9"
               OpName %v10 "v10"
               OpName %v11 "v11"
               OpName %v12 "v12"
               OpName %v13 "v13"
               OpName %v14 "v14"
               OpName %v15 "v15"
               OpName %v16 "v16"
               OpName %v17 "v17"
               OpName %v18 "v18"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %62 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %197 RelaxedPrecision
               OpDecorate %199 RelaxedPrecision
               OpDecorate %200 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
    %v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
        %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
%_ptr_Function_v4float = OpTypePointer Function %v4float
     %v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
     %v2bool = OpTypeVector %bool 2
%_ptr_Function_v2bool = OpTypePointer Function %v2bool
     %v3bool = OpTypeVector %bool 3
%_ptr_Function_v3bool = OpTypePointer Function %v3bool
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
         %38 = OpTypeFunction %bool %_ptr_Function_v2float %_ptr_Function_v2float %_ptr_Function_v2float %_ptr_Function_v3float %_ptr_Function_v2int %_ptr_Function_v2int %_ptr_Function_v2float %_ptr_Function_v2float %_ptr_Function_v4float %_ptr_Function_v2int %_ptr_Function_v4bool %_ptr_Function_v2float %_ptr_Function_v2float %_ptr_Function_v2float %_ptr_Function_v2bool %_ptr_Function_v2bool %_ptr_Function_v3bool %_ptr_Function_v4int
    %float_1 = OpConstant %float 1
   %float_18 = OpConstant %float 18
        %122 = OpTypeFunction %v4float %_ptr_Function_v2float
        %126 = OpConstantComposite %v2float %float_1 %float_1
    %float_2 = OpConstant %float 2
        %129 = OpConstantComposite %v2float %float_1 %float_2
        %132 = OpConstantComposite %v3float %float_1 %float_1 %float_1
      %int_1 = OpConstant %int 1
        %135 = OpConstantComposite %v2int %int_1 %int_1
      %int_2 = OpConstant %int 2
        %138 = OpConstantComposite %v2int %int_1 %int_2
%_ptr_Uniform_float = OpTypePointer Uniform %float
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
      %int_3 = OpConstant %int 3
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
        %159 = OpConstantComposite %v4bool %true %false %true %false
        %161 = OpConstantComposite %v2float %float_1 %float_0
        %165 = OpConstantComposite %v2bool %true %true
        %168 = OpConstantComposite %v3bool %true %true %true
        %170 = OpConstantComposite %v4int %int_1 %int_1 %int_1 %int_1
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %16
         %17 = OpLabel
         %21 = OpVariable %_ptr_Function_v2float Function
               OpStore %21 %20
         %23 = OpFunctionCall %v4float %main %21
               OpStore %sk_FragColor %23
               OpReturn
               OpFunctionEnd
%check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3i4 = OpFunction %bool None %38
         %39 = OpFunctionParameter %_ptr_Function_v2float
         %40 = OpFunctionParameter %_ptr_Function_v2float
         %41 = OpFunctionParameter %_ptr_Function_v2float
         %42 = OpFunctionParameter %_ptr_Function_v3float
         %43 = OpFunctionParameter %_ptr_Function_v2int
         %44 = OpFunctionParameter %_ptr_Function_v2int
         %45 = OpFunctionParameter %_ptr_Function_v2float
         %46 = OpFunctionParameter %_ptr_Function_v2float
         %47 = OpFunctionParameter %_ptr_Function_v4float
         %48 = OpFunctionParameter %_ptr_Function_v2int
         %49 = OpFunctionParameter %_ptr_Function_v4bool
         %50 = OpFunctionParameter %_ptr_Function_v2float
         %51 = OpFunctionParameter %_ptr_Function_v2float
         %52 = OpFunctionParameter %_ptr_Function_v2float
         %53 = OpFunctionParameter %_ptr_Function_v2bool
         %54 = OpFunctionParameter %_ptr_Function_v2bool
         %55 = OpFunctionParameter %_ptr_Function_v3bool
         %56 = OpFunctionParameter %_ptr_Function_v4int
         %57 = OpLabel
         %58 = OpLoad %v2float %39
         %59 = OpCompositeExtract %float %58 0
         %60 = OpLoad %v2float %40
         %61 = OpCompositeExtract %float %60 0
         %62 = OpFAdd %float %59 %61
         %63 = OpLoad %v2float %41
         %64 = OpCompositeExtract %float %63 0
         %65 = OpFAdd %float %62 %64
         %66 = OpLoad %v3float %42
         %67 = OpCompositeExtract %float %66 0
         %68 = OpFAdd %float %65 %67
         %69 = OpLoad %v2int %43
         %70 = OpCompositeExtract %int %69 0
         %71 = OpConvertSToF %float %70
         %72 = OpFAdd %float %68 %71
         %73 = OpLoad %v2int %44
         %74 = OpCompositeExtract %int %73 0
         %75 = OpConvertSToF %float %74
         %76 = OpFAdd %float %72 %75
         %77 = OpLoad %v2float %45
         %78 = OpCompositeExtract %float %77 0
         %79 = OpFAdd %float %76 %78
         %80 = OpLoad %v2float %46
         %81 = OpCompositeExtract %float %80 0
         %82 = OpFAdd %float %79 %81
         %83 = OpLoad %v4float %47
         %84 = OpCompositeExtract %float %83 0
         %85 = OpFAdd %float %82 %84
         %86 = OpLoad %v2int %48
         %87 = OpCompositeExtract %int %86 0
         %88 = OpConvertSToF %float %87
         %89 = OpFAdd %float %85 %88
         %90 = OpLoad %v4bool %49
         %91 = OpCompositeExtract %bool %90 0
         %92 = OpSelect %float %91 %float_1 %float_0
         %94 = OpFAdd %float %89 %92
         %95 = OpLoad %v2float %50
         %96 = OpCompositeExtract %float %95 0
         %97 = OpFAdd %float %94 %96
         %98 = OpLoad %v2float %51
         %99 = OpCompositeExtract %float %98 0
        %100 = OpFAdd %float %97 %99
        %101 = OpLoad %v2float %52
        %102 = OpCompositeExtract %float %101 0
        %103 = OpFAdd %float %100 %102
        %104 = OpLoad %v2bool %53
        %105 = OpCompositeExtract %bool %104 0
        %106 = OpSelect %float %105 %float_1 %float_0
        %107 = OpFAdd %float %103 %106
        %108 = OpLoad %v2bool %54
        %109 = OpCompositeExtract %bool %108 0
        %110 = OpSelect %float %109 %float_1 %float_0
        %111 = OpFAdd %float %107 %110
        %112 = OpLoad %v3bool %55
        %113 = OpCompositeExtract %bool %112 0
        %114 = OpSelect %float %113 %float_1 %float_0
        %115 = OpFAdd %float %111 %114
        %116 = OpLoad %v4int %56
        %117 = OpCompositeExtract %int %116 0
        %118 = OpConvertSToF %float %117
        %119 = OpFAdd %float %115 %118
        %121 = OpFOrdEqual %bool %119 %float_18
               OpReturnValue %121
               OpFunctionEnd
       %main = OpFunction %v4float None %122
        %123 = OpFunctionParameter %_ptr_Function_v2float
        %124 = OpLabel
         %v1 = OpVariable %_ptr_Function_v2float Function
         %v2 = OpVariable %_ptr_Function_v2float Function
         %v3 = OpVariable %_ptr_Function_v2float Function
         %v4 = OpVariable %_ptr_Function_v3float Function
         %v5 = OpVariable %_ptr_Function_v2int Function
         %v6 = OpVariable %_ptr_Function_v2int Function
         %v7 = OpVariable %_ptr_Function_v2float Function
         %v8 = OpVariable %_ptr_Function_v2float Function
         %v9 = OpVariable %_ptr_Function_v4float Function
        %v10 = OpVariable %_ptr_Function_v2int Function
        %v11 = OpVariable %_ptr_Function_v4bool Function
        %v12 = OpVariable %_ptr_Function_v2float Function
        %v13 = OpVariable %_ptr_Function_v2float Function
        %v14 = OpVariable %_ptr_Function_v2float Function
        %v15 = OpVariable %_ptr_Function_v2bool Function
        %v16 = OpVariable %_ptr_Function_v2bool Function
        %v17 = OpVariable %_ptr_Function_v3bool Function
        %v18 = OpVariable %_ptr_Function_v4int Function
        %171 = OpVariable %_ptr_Function_v2float Function
        %172 = OpVariable %_ptr_Function_v2float Function
        %173 = OpVariable %_ptr_Function_v2float Function
        %174 = OpVariable %_ptr_Function_v3float Function
        %175 = OpVariable %_ptr_Function_v2int Function
        %176 = OpVariable %_ptr_Function_v2int Function
        %177 = OpVariable %_ptr_Function_v2float Function
        %178 = OpVariable %_ptr_Function_v2float Function
        %179 = OpVariable %_ptr_Function_v4float Function
        %180 = OpVariable %_ptr_Function_v2int Function
        %181 = OpVariable %_ptr_Function_v4bool Function
        %182 = OpVariable %_ptr_Function_v2float Function
        %183 = OpVariable %_ptr_Function_v2float Function
        %184 = OpVariable %_ptr_Function_v2float Function
        %185 = OpVariable %_ptr_Function_v2bool Function
        %186 = OpVariable %_ptr_Function_v2bool Function
        %187 = OpVariable %_ptr_Function_v3bool Function
        %188 = OpVariable %_ptr_Function_v4int Function
        %190 = OpVariable %_ptr_Function_v4float Function
               OpStore %v1 %126
               OpStore %v2 %129
               OpStore %v3 %126
               OpStore %v4 %132
               OpStore %v5 %135
               OpStore %v6 %138
               OpStore %v7 %129
        %141 = OpConvertSToF %float %int_1
        %142 = OpConvertSToF %float %int_1
        %143 = OpCompositeConstruct %v2float %141 %142
               OpStore %v8 %143
        %145 = OpConvertSToF %float %int_1
        %146 = OpAccessChain %_ptr_Uniform_float %11 %int_2
        %148 = OpLoad %float %146
        %151 = OpCompositeConstruct %v4float %145 %148 %float_3 %float_4
               OpStore %v9 %151
        %154 = OpConvertFToS %int %float_1
        %155 = OpCompositeConstruct %v2int %int_3 %154
               OpStore %v10 %155
               OpStore %v11 %159
               OpStore %v12 %161
               OpStore %v13 %20
               OpStore %v14 %20
               OpStore %v15 %165
               OpStore %v16 %165
               OpStore %v17 %168
               OpStore %v18 %170
               OpStore %171 %126
               OpStore %172 %129
               OpStore %173 %126
               OpStore %174 %132
               OpStore %175 %135
               OpStore %176 %138
               OpStore %177 %129
               OpStore %178 %143
               OpStore %179 %151
               OpStore %180 %155
               OpStore %181 %159
               OpStore %182 %161
               OpStore %183 %20
               OpStore %184 %20
               OpStore %185 %165
               OpStore %186 %165
               OpStore %187 %168
               OpStore %188 %170
        %189 = OpFunctionCall %bool %check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3i4 %171 %172 %173 %174 %175 %176 %177 %178 %179 %180 %181 %182 %183 %184 %185 %186 %187 %188
               OpSelectionMerge %193 None
               OpBranchConditional %189 %191 %192
        %191 = OpLabel
        %194 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %197 = OpLoad %v4float %194
               OpStore %190 %197
               OpBranch %193
        %192 = OpLabel
        %198 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
        %199 = OpLoad %v4float %198
               OpStore %190 %199
               OpBranch %193
        %193 = OpLabel
        %200 = OpLoad %v4float %190
               OpReturnValue %200
               OpFunctionEnd
