               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
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
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpDecorate %_UniformBuffer Block
               OpDecorate %8 Binding 0
               OpDecorate %8 DescriptorSet 0
               OpDecorate %60 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %116 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %195 RelaxedPrecision
               OpDecorate %197 RelaxedPrecision
               OpDecorate %198 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %float
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
         %36 = OpTypeFunction %bool %_ptr_Function_v2float %_ptr_Function_v2float %_ptr_Function_v2float %_ptr_Function_v3float %_ptr_Function_v2int %_ptr_Function_v2int %_ptr_Function_v2float %_ptr_Function_v2float %_ptr_Function_v4float %_ptr_Function_v2int %_ptr_Function_v4bool %_ptr_Function_v2float %_ptr_Function_v2float %_ptr_Function_v2float %_ptr_Function_v2bool %_ptr_Function_v2bool %_ptr_Function_v3bool %_ptr_Function_v4int
    %float_1 = OpConstant %float 1
   %float_18 = OpConstant %float 18
        %120 = OpTypeFunction %v4float %_ptr_Function_v2float
        %124 = OpConstantComposite %v2float %float_1 %float_1
    %float_2 = OpConstant %float 2
        %127 = OpConstantComposite %v2float %float_1 %float_2
        %130 = OpConstantComposite %v3float %float_1 %float_1 %float_1
      %int_1 = OpConstant %int 1
        %133 = OpConstantComposite %v2int %int_1 %int_1
      %int_2 = OpConstant %int 2
        %136 = OpConstantComposite %v2int %int_1 %int_2
%_ptr_Uniform_float = OpTypePointer Uniform %float
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
      %int_3 = OpConstant %int 3
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
        %157 = OpConstantComposite %v4bool %true %false %true %false
        %159 = OpConstantComposite %v2float %float_1 %float_0
        %163 = OpConstantComposite %v2bool %true %true
        %166 = OpConstantComposite %v3bool %true %true %true
        %168 = OpConstantComposite %v4int %int_1 %int_1 %int_1 %int_1
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %13
         %14 = OpLabel
         %18 = OpVariable %_ptr_Function_v2float Function
               OpStore %18 %17
         %20 = OpFunctionCall %v4float %main %18
               OpStore %sk_FragColor %20
               OpReturn
               OpFunctionEnd
%check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3i4 = OpFunction %bool None %36
         %37 = OpFunctionParameter %_ptr_Function_v2float
         %38 = OpFunctionParameter %_ptr_Function_v2float
         %39 = OpFunctionParameter %_ptr_Function_v2float
         %40 = OpFunctionParameter %_ptr_Function_v3float
         %41 = OpFunctionParameter %_ptr_Function_v2int
         %42 = OpFunctionParameter %_ptr_Function_v2int
         %43 = OpFunctionParameter %_ptr_Function_v2float
         %44 = OpFunctionParameter %_ptr_Function_v2float
         %45 = OpFunctionParameter %_ptr_Function_v4float
         %46 = OpFunctionParameter %_ptr_Function_v2int
         %47 = OpFunctionParameter %_ptr_Function_v4bool
         %48 = OpFunctionParameter %_ptr_Function_v2float
         %49 = OpFunctionParameter %_ptr_Function_v2float
         %50 = OpFunctionParameter %_ptr_Function_v2float
         %51 = OpFunctionParameter %_ptr_Function_v2bool
         %52 = OpFunctionParameter %_ptr_Function_v2bool
         %53 = OpFunctionParameter %_ptr_Function_v3bool
         %54 = OpFunctionParameter %_ptr_Function_v4int
         %55 = OpLabel
         %56 = OpLoad %v2float %37
         %57 = OpCompositeExtract %float %56 0
         %58 = OpLoad %v2float %38
         %59 = OpCompositeExtract %float %58 0
         %60 = OpFAdd %float %57 %59
         %61 = OpLoad %v2float %39
         %62 = OpCompositeExtract %float %61 0
         %63 = OpFAdd %float %60 %62
         %64 = OpLoad %v3float %40
         %65 = OpCompositeExtract %float %64 0
         %66 = OpFAdd %float %63 %65
         %67 = OpLoad %v2int %41
         %68 = OpCompositeExtract %int %67 0
         %69 = OpConvertSToF %float %68
         %70 = OpFAdd %float %66 %69
         %71 = OpLoad %v2int %42
         %72 = OpCompositeExtract %int %71 0
         %73 = OpConvertSToF %float %72
         %74 = OpFAdd %float %70 %73
         %75 = OpLoad %v2float %43
         %76 = OpCompositeExtract %float %75 0
         %77 = OpFAdd %float %74 %76
         %78 = OpLoad %v2float %44
         %79 = OpCompositeExtract %float %78 0
         %80 = OpFAdd %float %77 %79
         %81 = OpLoad %v4float %45
         %82 = OpCompositeExtract %float %81 0
         %83 = OpFAdd %float %80 %82
         %84 = OpLoad %v2int %46
         %85 = OpCompositeExtract %int %84 0
         %86 = OpConvertSToF %float %85
         %87 = OpFAdd %float %83 %86
         %88 = OpLoad %v4bool %47
         %89 = OpCompositeExtract %bool %88 0
         %90 = OpSelect %float %89 %float_1 %float_0
         %92 = OpFAdd %float %87 %90
         %93 = OpLoad %v2float %48
         %94 = OpCompositeExtract %float %93 0
         %95 = OpFAdd %float %92 %94
         %96 = OpLoad %v2float %49
         %97 = OpCompositeExtract %float %96 0
         %98 = OpFAdd %float %95 %97
         %99 = OpLoad %v2float %50
        %100 = OpCompositeExtract %float %99 0
        %101 = OpFAdd %float %98 %100
        %102 = OpLoad %v2bool %51
        %103 = OpCompositeExtract %bool %102 0
        %104 = OpSelect %float %103 %float_1 %float_0
        %105 = OpFAdd %float %101 %104
        %106 = OpLoad %v2bool %52
        %107 = OpCompositeExtract %bool %106 0
        %108 = OpSelect %float %107 %float_1 %float_0
        %109 = OpFAdd %float %105 %108
        %110 = OpLoad %v3bool %53
        %111 = OpCompositeExtract %bool %110 0
        %112 = OpSelect %float %111 %float_1 %float_0
        %113 = OpFAdd %float %109 %112
        %114 = OpLoad %v4int %54
        %115 = OpCompositeExtract %int %114 0
        %116 = OpConvertSToF %float %115
        %117 = OpFAdd %float %113 %116
        %119 = OpFOrdEqual %bool %117 %float_18
               OpReturnValue %119
               OpFunctionEnd
       %main = OpFunction %v4float None %120
        %121 = OpFunctionParameter %_ptr_Function_v2float
        %122 = OpLabel
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
        %169 = OpVariable %_ptr_Function_v2float Function
        %170 = OpVariable %_ptr_Function_v2float Function
        %171 = OpVariable %_ptr_Function_v2float Function
        %172 = OpVariable %_ptr_Function_v3float Function
        %173 = OpVariable %_ptr_Function_v2int Function
        %174 = OpVariable %_ptr_Function_v2int Function
        %175 = OpVariable %_ptr_Function_v2float Function
        %176 = OpVariable %_ptr_Function_v2float Function
        %177 = OpVariable %_ptr_Function_v4float Function
        %178 = OpVariable %_ptr_Function_v2int Function
        %179 = OpVariable %_ptr_Function_v4bool Function
        %180 = OpVariable %_ptr_Function_v2float Function
        %181 = OpVariable %_ptr_Function_v2float Function
        %182 = OpVariable %_ptr_Function_v2float Function
        %183 = OpVariable %_ptr_Function_v2bool Function
        %184 = OpVariable %_ptr_Function_v2bool Function
        %185 = OpVariable %_ptr_Function_v3bool Function
        %186 = OpVariable %_ptr_Function_v4int Function
        %188 = OpVariable %_ptr_Function_v4float Function
               OpStore %v1 %124
               OpStore %v2 %127
               OpStore %v3 %124
               OpStore %v4 %130
               OpStore %v5 %133
               OpStore %v6 %136
               OpStore %v7 %127
        %139 = OpConvertSToF %float %int_1
        %140 = OpConvertSToF %float %int_1
        %141 = OpCompositeConstruct %v2float %139 %140
               OpStore %v8 %141
        %143 = OpConvertSToF %float %int_1
        %144 = OpAccessChain %_ptr_Uniform_float %8 %int_2
        %146 = OpLoad %float %144
        %149 = OpCompositeConstruct %v4float %143 %146 %float_3 %float_4
               OpStore %v9 %149
        %152 = OpConvertFToS %int %float_1
        %153 = OpCompositeConstruct %v2int %int_3 %152
               OpStore %v10 %153
               OpStore %v11 %157
               OpStore %v12 %159
               OpStore %v13 %17
               OpStore %v14 %17
               OpStore %v15 %163
               OpStore %v16 %163
               OpStore %v17 %166
               OpStore %v18 %168
               OpStore %169 %124
               OpStore %170 %127
               OpStore %171 %124
               OpStore %172 %130
               OpStore %173 %133
               OpStore %174 %136
               OpStore %175 %127
               OpStore %176 %141
               OpStore %177 %149
               OpStore %178 %153
               OpStore %179 %157
               OpStore %180 %159
               OpStore %181 %17
               OpStore %182 %17
               OpStore %183 %163
               OpStore %184 %163
               OpStore %185 %166
               OpStore %186 %168
        %187 = OpFunctionCall %bool %check_bf2f2f2f3i2i2f2f2f4i2b4f2f2f2b2b2b3i4 %169 %170 %171 %172 %173 %174 %175 %176 %177 %178 %179 %180 %181 %182 %183 %184 %185 %186
               OpSelectionMerge %191 None
               OpBranchConditional %187 %189 %190
        %189 = OpLabel
        %192 = OpAccessChain %_ptr_Uniform_v4float %8 %int_0
        %195 = OpLoad %v4float %192
               OpStore %188 %195
               OpBranch %191
        %190 = OpLabel
        %196 = OpAccessChain %_ptr_Uniform_v4float %8 %int_1
        %197 = OpLoad %v4float %196
               OpStore %188 %197
               OpBranch %191
        %191 = OpLabel
        %198 = OpLoad %v4float %188
               OpReturnValue %198
               OpFunctionEnd
