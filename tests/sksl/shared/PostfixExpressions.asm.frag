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
               OpName %main "main"
               OpName %ok "ok"
               OpName %i "i"
               OpName %f "f"
               OpName %f2 "f2"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %103 RelaxedPrecision
               OpDecorate %133 RelaxedPrecision
               OpDecorate %147 RelaxedPrecision
               OpDecorate %149 RelaxedPrecision
               OpDecorate %150 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
       %bool = OpTypeBool
%_ptr_Function_bool = OpTypePointer Function %bool
       %true = OpConstantTrue %bool
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
      %int_5 = OpConstant %int 5
      %int_1 = OpConstant %int 1
      %false = OpConstantFalse %bool
      %int_6 = OpConstant %int 6
      %int_7 = OpConstant %int 7
%_ptr_Function_float = OpTypePointer Function %float
  %float_0_5 = OpConstant %float 0.5
    %float_1 = OpConstant %float 1
  %float_1_5 = OpConstant %float 1.5
  %float_2_5 = OpConstant %float 2.5
         %98 = OpConstantComposite %v2float %float_0_5 %float_0_5
      %int_0 = OpConstant %int 0
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %12
         %13 = OpLabel
         %17 = OpVariable %_ptr_Function_v2float Function
               OpStore %17 %16
         %19 = OpFunctionCall %v4float %main %17
               OpStore %sk_FragColor %19
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %20
         %21 = OpFunctionParameter %_ptr_Function_v2float
         %22 = OpLabel
         %ok = OpVariable %_ptr_Function_bool Function
          %i = OpVariable %_ptr_Function_int Function
          %f = OpVariable %_ptr_Function_float Function
         %f2 = OpVariable %_ptr_Function_v2float Function
        %140 = OpVariable %_ptr_Function_v4float Function
               OpStore %ok %true
               OpStore %i %int_5
         %32 = OpIAdd %int %int_5 %int_1
               OpStore %i %32
               OpSelectionMerge %35 None
               OpBranchConditional %true %34 %35
         %34 = OpLabel
         %36 = OpIAdd %int %32 %int_1
               OpStore %i %36
         %38 = OpIEqual %bool %32 %int_6
               OpBranch %35
         %35 = OpLabel
         %39 = OpPhi %bool %false %22 %38 %34
               OpStore %ok %39
               OpSelectionMerge %41 None
               OpBranchConditional %39 %40 %41
         %40 = OpLabel
         %42 = OpLoad %int %i
         %44 = OpIEqual %bool %42 %int_7
               OpBranch %41
         %41 = OpLabel
         %45 = OpPhi %bool %false %35 %44 %40
               OpStore %ok %45
               OpSelectionMerge %47 None
               OpBranchConditional %45 %46 %47
         %46 = OpLabel
         %48 = OpLoad %int %i
         %49 = OpISub %int %48 %int_1
               OpStore %i %49
         %50 = OpIEqual %bool %48 %int_7
               OpBranch %47
         %47 = OpLabel
         %51 = OpPhi %bool %false %41 %50 %46
               OpStore %ok %51
               OpSelectionMerge %53 None
               OpBranchConditional %51 %52 %53
         %52 = OpLabel
         %54 = OpLoad %int %i
         %55 = OpIEqual %bool %54 %int_6
               OpBranch %53
         %53 = OpLabel
         %56 = OpPhi %bool %false %47 %55 %52
               OpStore %ok %56
         %57 = OpLoad %int %i
         %58 = OpISub %int %57 %int_1
               OpStore %i %58
               OpSelectionMerge %60 None
               OpBranchConditional %56 %59 %60
         %59 = OpLabel
         %61 = OpIEqual %bool %58 %int_5
               OpBranch %60
         %60 = OpLabel
         %62 = OpPhi %bool %false %53 %61 %59
               OpStore %ok %62
               OpStore %f %float_0_5
         %67 = OpFAdd %float %float_0_5 %float_1
               OpStore %f %67
               OpSelectionMerge %69 None
               OpBranchConditional %62 %68 %69
         %68 = OpLabel
         %70 = OpFAdd %float %67 %float_1
               OpStore %f %70
         %72 = OpFOrdEqual %bool %67 %float_1_5
               OpBranch %69
         %69 = OpLabel
         %73 = OpPhi %bool %false %60 %72 %68
               OpStore %ok %73
               OpSelectionMerge %75 None
               OpBranchConditional %73 %74 %75
         %74 = OpLabel
         %76 = OpLoad %float %f
         %78 = OpFOrdEqual %bool %76 %float_2_5
               OpBranch %75
         %75 = OpLabel
         %79 = OpPhi %bool %false %69 %78 %74
               OpStore %ok %79
               OpSelectionMerge %81 None
               OpBranchConditional %79 %80 %81
         %80 = OpLabel
         %82 = OpLoad %float %f
         %83 = OpFSub %float %82 %float_1
               OpStore %f %83
         %84 = OpFOrdEqual %bool %82 %float_2_5
               OpBranch %81
         %81 = OpLabel
         %85 = OpPhi %bool %false %75 %84 %80
               OpStore %ok %85
               OpSelectionMerge %87 None
               OpBranchConditional %85 %86 %87
         %86 = OpLabel
         %88 = OpLoad %float %f
         %89 = OpFOrdEqual %bool %88 %float_1_5
               OpBranch %87
         %87 = OpLabel
         %90 = OpPhi %bool %false %81 %89 %86
               OpStore %ok %90
         %91 = OpLoad %float %f
         %92 = OpFSub %float %91 %float_1
               OpStore %f %92
               OpSelectionMerge %94 None
               OpBranchConditional %90 %93 %94
         %93 = OpLabel
         %95 = OpFOrdEqual %bool %92 %float_0_5
               OpBranch %94
         %94 = OpLabel
         %96 = OpPhi %bool %false %87 %95 %93
               OpStore %ok %96
               OpStore %f2 %98
         %99 = OpAccessChain %_ptr_Function_float %f2 %int_0
        %101 = OpLoad %float %99
        %102 = OpFAdd %float %101 %float_1
               OpStore %99 %102
        %103 = OpLoad %bool %ok
               OpSelectionMerge %105 None
               OpBranchConditional %103 %104 %105
        %104 = OpLabel
        %106 = OpAccessChain %_ptr_Function_float %f2 %int_0
        %107 = OpLoad %float %106
        %108 = OpFAdd %float %107 %float_1
               OpStore %106 %108
        %109 = OpFOrdEqual %bool %107 %float_1_5
               OpBranch %105
        %105 = OpLabel
        %110 = OpPhi %bool %false %94 %109 %104
               OpStore %ok %110
               OpSelectionMerge %112 None
               OpBranchConditional %110 %111 %112
        %111 = OpLabel
        %113 = OpLoad %v2float %f2
        %114 = OpCompositeExtract %float %113 0
        %115 = OpFOrdEqual %bool %114 %float_2_5
               OpBranch %112
        %112 = OpLabel
        %116 = OpPhi %bool %false %105 %115 %111
               OpStore %ok %116
               OpSelectionMerge %118 None
               OpBranchConditional %116 %117 %118
        %117 = OpLabel
        %119 = OpAccessChain %_ptr_Function_float %f2 %int_0
        %120 = OpLoad %float %119
        %121 = OpFSub %float %120 %float_1
               OpStore %119 %121
        %122 = OpFOrdEqual %bool %120 %float_2_5
               OpBranch %118
        %118 = OpLabel
        %123 = OpPhi %bool %false %112 %122 %117
               OpStore %ok %123
               OpSelectionMerge %125 None
               OpBranchConditional %123 %124 %125
        %124 = OpLabel
        %126 = OpLoad %v2float %f2
        %127 = OpCompositeExtract %float %126 0
        %128 = OpFOrdEqual %bool %127 %float_1_5
               OpBranch %125
        %125 = OpLabel
        %129 = OpPhi %bool %false %118 %128 %124
               OpStore %ok %129
        %130 = OpAccessChain %_ptr_Function_float %f2 %int_0
        %131 = OpLoad %float %130
        %132 = OpFSub %float %131 %float_1
               OpStore %130 %132
        %133 = OpLoad %bool %ok
               OpSelectionMerge %135 None
               OpBranchConditional %133 %134 %135
        %134 = OpLabel
        %136 = OpLoad %v2float %f2
        %137 = OpCompositeExtract %float %136 0
        %138 = OpFOrdEqual %bool %137 %float_0_5
               OpBranch %135
        %135 = OpLabel
        %139 = OpPhi %bool %false %125 %138 %134
               OpStore %ok %139
               OpSelectionMerge %144 None
               OpBranchConditional %139 %142 %143
        %142 = OpLabel
        %145 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %147 = OpLoad %v4float %145
               OpStore %140 %147
               OpBranch %144
        %143 = OpLabel
        %148 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %149 = OpLoad %v4float %148
               OpStore %140 %149
               OpBranch %144
        %144 = OpLabel
        %150 = OpLoad %v4float %140
               OpReturnValue %150
               OpFunctionEnd
