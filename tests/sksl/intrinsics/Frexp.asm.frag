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
               OpName %value "value"
               OpName %exp "exp"
               OpName %result "result"
               OpName %ok "ok"
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
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %125 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
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
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_6 = OpConstant %float 6
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
     %v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Function_float = OpTypePointer Function %float
      %false = OpConstantFalse %bool
 %float_0_75 = OpConstant %float 0.75
      %int_3 = OpConstant %int 3
%_ptr_Function_bool = OpTypePointer Function %bool
      %v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
      %int_1 = OpConstant %int 1
    %v3float = OpTypeVector %float 3
      %v3int = OpTypeVector %int 3
%_ptr_Function_v3int = OpTypePointer Function %v3int
      %int_2 = OpConstant %int 2
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
      %value = OpVariable %_ptr_Function_v4float Function
        %exp = OpVariable %_ptr_Function_v4int Function
     %result = OpVariable %_ptr_Function_v4float Function
         %ok = OpVariable %_ptr_Function_v4bool Function
         %47 = OpVariable %_ptr_Function_int Function
         %68 = OpVariable %_ptr_Function_v2int Function
         %89 = OpVariable %_ptr_Function_v3int Function
        %108 = OpVariable %_ptr_Function_v4int Function
        %120 = OpVariable %_ptr_Function_v4float Function
         %28 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %32 = OpLoad %v4float %28
         %33 = OpVectorShuffle %v4float %32 %32 1 1 1 1
         %35 = OpVectorTimesScalar %v4float %33 %float_6
               OpStore %value %35
         %44 = OpCompositeExtract %float %35 0
         %45 = OpAccessChain %_ptr_Function_int %exp %int_0
         %43 = OpExtInst %float %1 Frexp %44 %47
         %48 = OpLoad %int %47
               OpStore %45 %48
         %49 = OpAccessChain %_ptr_Function_float %result %int_0
               OpStore %49 %43
         %52 = OpLoad %v4float %result
         %53 = OpCompositeExtract %float %52 0
         %55 = OpFOrdEqual %bool %53 %float_0_75
               OpSelectionMerge %57 None
               OpBranchConditional %55 %56 %57
         %56 = OpLabel
         %58 = OpLoad %v4int %exp
         %59 = OpCompositeExtract %int %58 0
         %61 = OpIEqual %bool %59 %int_3
               OpBranch %57
         %57 = OpLabel
         %62 = OpPhi %bool %false %25 %61 %56
         %63 = OpAccessChain %_ptr_Function_bool %ok %int_0
               OpStore %63 %62
         %66 = OpLoad %v4float %value
         %67 = OpVectorShuffle %v2float %66 %66 0 1
         %65 = OpExtInst %v2float %1 Frexp %67 %68
         %71 = OpLoad %v2int %68
         %72 = OpLoad %v4int %exp
         %73 = OpVectorShuffle %v4int %72 %71 4 5 2 3
               OpStore %exp %73
         %74 = OpLoad %v4float %result
         %75 = OpVectorShuffle %v4float %74 %65 4 5 2 3
               OpStore %result %75
         %76 = OpCompositeExtract %float %75 1
         %77 = OpFOrdEqual %bool %76 %float_0_75
               OpSelectionMerge %79 None
               OpBranchConditional %77 %78 %79
         %78 = OpLabel
         %80 = OpCompositeExtract %int %73 1
         %81 = OpIEqual %bool %80 %int_3
               OpBranch %79
         %79 = OpLabel
         %82 = OpPhi %bool %false %57 %81 %78
         %83 = OpAccessChain %_ptr_Function_bool %ok %int_1
               OpStore %83 %82
         %86 = OpLoad %v4float %value
         %87 = OpVectorShuffle %v3float %86 %86 0 1 2
         %85 = OpExtInst %v3float %1 Frexp %87 %89
         %92 = OpLoad %v3int %89
         %93 = OpLoad %v4int %exp
         %94 = OpVectorShuffle %v4int %93 %92 4 5 6 3
               OpStore %exp %94
         %95 = OpLoad %v4float %result
         %96 = OpVectorShuffle %v4float %95 %85 4 5 6 3
               OpStore %result %96
         %97 = OpCompositeExtract %float %96 2
         %98 = OpFOrdEqual %bool %97 %float_0_75
               OpSelectionMerge %100 None
               OpBranchConditional %98 %99 %100
         %99 = OpLabel
        %101 = OpCompositeExtract %int %94 2
        %102 = OpIEqual %bool %101 %int_3
               OpBranch %100
        %100 = OpLabel
        %103 = OpPhi %bool %false %79 %102 %99
        %104 = OpAccessChain %_ptr_Function_bool %ok %int_2
               OpStore %104 %103
        %107 = OpLoad %v4float %value
        %106 = OpExtInst %v4float %1 Frexp %107 %108
        %109 = OpLoad %v4int %108
               OpStore %exp %109
               OpStore %result %106
        %110 = OpCompositeExtract %float %106 3
        %111 = OpFOrdEqual %bool %110 %float_0_75
               OpSelectionMerge %113 None
               OpBranchConditional %111 %112 %113
        %112 = OpLabel
        %114 = OpCompositeExtract %int %109 3
        %115 = OpIEqual %bool %114 %int_3
               OpBranch %113
        %113 = OpLabel
        %116 = OpPhi %bool %false %100 %115 %112
        %117 = OpAccessChain %_ptr_Function_bool %ok %int_3
               OpStore %117 %116
        %119 = OpLoad %v4bool %ok
        %118 = OpAll %bool %119
               OpSelectionMerge %123 None
               OpBranchConditional %118 %121 %122
        %121 = OpLabel
        %124 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %125 = OpLoad %v4float %124
               OpStore %120 %125
               OpBranch %123
        %122 = OpLabel
        %126 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %127 = OpLoad %v4float %126
               OpStore %120 %127
               OpBranch %123
        %123 = OpLabel
        %128 = OpLoad %v4float %120
               OpReturnValue %128
               OpFunctionEnd
