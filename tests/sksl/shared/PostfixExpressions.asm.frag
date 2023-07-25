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
               OpName %ok "ok"
               OpName %i "i"
               OpName %f "f"
               OpName %f2 "f2"
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
               OpDecorate %105 RelaxedPrecision
               OpDecorate %135 RelaxedPrecision
               OpDecorate %149 RelaxedPrecision
               OpDecorate %151 RelaxedPrecision
               OpDecorate %152 RelaxedPrecision
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
        %100 = OpConstantComposite %v2float %float_0_5 %float_0_5
      %int_0 = OpConstant %int 0
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
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
         %ok = OpVariable %_ptr_Function_bool Function
          %i = OpVariable %_ptr_Function_int Function
          %f = OpVariable %_ptr_Function_float Function
         %f2 = OpVariable %_ptr_Function_v2float Function
        %142 = OpVariable %_ptr_Function_v4float Function
               OpStore %ok %true
               OpStore %i %int_5
         %34 = OpIAdd %int %int_5 %int_1
               OpStore %i %34
               OpSelectionMerge %37 None
               OpBranchConditional %true %36 %37
         %36 = OpLabel
         %38 = OpIAdd %int %34 %int_1
               OpStore %i %38
         %40 = OpIEqual %bool %34 %int_6
               OpBranch %37
         %37 = OpLabel
         %41 = OpPhi %bool %false %25 %40 %36
               OpStore %ok %41
               OpSelectionMerge %43 None
               OpBranchConditional %41 %42 %43
         %42 = OpLabel
         %44 = OpLoad %int %i
         %46 = OpIEqual %bool %44 %int_7
               OpBranch %43
         %43 = OpLabel
         %47 = OpPhi %bool %false %37 %46 %42
               OpStore %ok %47
               OpSelectionMerge %49 None
               OpBranchConditional %47 %48 %49
         %48 = OpLabel
         %50 = OpLoad %int %i
         %51 = OpISub %int %50 %int_1
               OpStore %i %51
         %52 = OpIEqual %bool %50 %int_7
               OpBranch %49
         %49 = OpLabel
         %53 = OpPhi %bool %false %43 %52 %48
               OpStore %ok %53
               OpSelectionMerge %55 None
               OpBranchConditional %53 %54 %55
         %54 = OpLabel
         %56 = OpLoad %int %i
         %57 = OpIEqual %bool %56 %int_6
               OpBranch %55
         %55 = OpLabel
         %58 = OpPhi %bool %false %49 %57 %54
               OpStore %ok %58
         %59 = OpLoad %int %i
         %60 = OpISub %int %59 %int_1
               OpStore %i %60
               OpSelectionMerge %62 None
               OpBranchConditional %58 %61 %62
         %61 = OpLabel
         %63 = OpIEqual %bool %60 %int_5
               OpBranch %62
         %62 = OpLabel
         %64 = OpPhi %bool %false %55 %63 %61
               OpStore %ok %64
               OpStore %f %float_0_5
         %69 = OpFAdd %float %float_0_5 %float_1
               OpStore %f %69
               OpSelectionMerge %71 None
               OpBranchConditional %64 %70 %71
         %70 = OpLabel
         %72 = OpFAdd %float %69 %float_1
               OpStore %f %72
         %74 = OpFOrdEqual %bool %69 %float_1_5
               OpBranch %71
         %71 = OpLabel
         %75 = OpPhi %bool %false %62 %74 %70
               OpStore %ok %75
               OpSelectionMerge %77 None
               OpBranchConditional %75 %76 %77
         %76 = OpLabel
         %78 = OpLoad %float %f
         %80 = OpFOrdEqual %bool %78 %float_2_5
               OpBranch %77
         %77 = OpLabel
         %81 = OpPhi %bool %false %71 %80 %76
               OpStore %ok %81
               OpSelectionMerge %83 None
               OpBranchConditional %81 %82 %83
         %82 = OpLabel
         %84 = OpLoad %float %f
         %85 = OpFSub %float %84 %float_1
               OpStore %f %85
         %86 = OpFOrdEqual %bool %84 %float_2_5
               OpBranch %83
         %83 = OpLabel
         %87 = OpPhi %bool %false %77 %86 %82
               OpStore %ok %87
               OpSelectionMerge %89 None
               OpBranchConditional %87 %88 %89
         %88 = OpLabel
         %90 = OpLoad %float %f
         %91 = OpFOrdEqual %bool %90 %float_1_5
               OpBranch %89
         %89 = OpLabel
         %92 = OpPhi %bool %false %83 %91 %88
               OpStore %ok %92
         %93 = OpLoad %float %f
         %94 = OpFSub %float %93 %float_1
               OpStore %f %94
               OpSelectionMerge %96 None
               OpBranchConditional %92 %95 %96
         %95 = OpLabel
         %97 = OpFOrdEqual %bool %94 %float_0_5
               OpBranch %96
         %96 = OpLabel
         %98 = OpPhi %bool %false %89 %97 %95
               OpStore %ok %98
               OpStore %f2 %100
        %101 = OpAccessChain %_ptr_Function_float %f2 %int_0
        %103 = OpLoad %float %101
        %104 = OpFAdd %float %103 %float_1
               OpStore %101 %104
        %105 = OpLoad %bool %ok
               OpSelectionMerge %107 None
               OpBranchConditional %105 %106 %107
        %106 = OpLabel
        %108 = OpAccessChain %_ptr_Function_float %f2 %int_0
        %109 = OpLoad %float %108
        %110 = OpFAdd %float %109 %float_1
               OpStore %108 %110
        %111 = OpFOrdEqual %bool %109 %float_1_5
               OpBranch %107
        %107 = OpLabel
        %112 = OpPhi %bool %false %96 %111 %106
               OpStore %ok %112
               OpSelectionMerge %114 None
               OpBranchConditional %112 %113 %114
        %113 = OpLabel
        %115 = OpLoad %v2float %f2
        %116 = OpCompositeExtract %float %115 0
        %117 = OpFOrdEqual %bool %116 %float_2_5
               OpBranch %114
        %114 = OpLabel
        %118 = OpPhi %bool %false %107 %117 %113
               OpStore %ok %118
               OpSelectionMerge %120 None
               OpBranchConditional %118 %119 %120
        %119 = OpLabel
        %121 = OpAccessChain %_ptr_Function_float %f2 %int_0
        %122 = OpLoad %float %121
        %123 = OpFSub %float %122 %float_1
               OpStore %121 %123
        %124 = OpFOrdEqual %bool %122 %float_2_5
               OpBranch %120
        %120 = OpLabel
        %125 = OpPhi %bool %false %114 %124 %119
               OpStore %ok %125
               OpSelectionMerge %127 None
               OpBranchConditional %125 %126 %127
        %126 = OpLabel
        %128 = OpLoad %v2float %f2
        %129 = OpCompositeExtract %float %128 0
        %130 = OpFOrdEqual %bool %129 %float_1_5
               OpBranch %127
        %127 = OpLabel
        %131 = OpPhi %bool %false %120 %130 %126
               OpStore %ok %131
        %132 = OpAccessChain %_ptr_Function_float %f2 %int_0
        %133 = OpLoad %float %132
        %134 = OpFSub %float %133 %float_1
               OpStore %132 %134
        %135 = OpLoad %bool %ok
               OpSelectionMerge %137 None
               OpBranchConditional %135 %136 %137
        %136 = OpLabel
        %138 = OpLoad %v2float %f2
        %139 = OpCompositeExtract %float %138 0
        %140 = OpFOrdEqual %bool %139 %float_0_5
               OpBranch %137
        %137 = OpLabel
        %141 = OpPhi %bool %false %127 %140 %136
               OpStore %ok %141
               OpSelectionMerge %146 None
               OpBranchConditional %141 %144 %145
        %144 = OpLabel
        %147 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %149 = OpLoad %v4float %147
               OpStore %142 %149
               OpBranch %146
        %145 = OpLabel
        %150 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %151 = OpLoad %v4float %150
               OpStore %142 %151
               OpBranch %146
        %146 = OpLabel
        %152 = OpLoad %v4float %142
               OpReturnValue %152
               OpFunctionEnd
