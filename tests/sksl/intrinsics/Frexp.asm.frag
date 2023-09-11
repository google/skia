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
               OpName %value "value"
               OpName %exp "exp"
               OpName %result "result"
               OpName %ok "ok"
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
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
               OpDecorate %125 RelaxedPrecision
               OpDecorate %126 RelaxedPrecision
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
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_6 = OpConstant %float 6
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
       %bool = OpTypeBool
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
      %value = OpVariable %_ptr_Function_v4float Function
        %exp = OpVariable %_ptr_Function_v4int Function
     %result = OpVariable %_ptr_Function_v4float Function
         %ok = OpVariable %_ptr_Function_v4bool Function
         %45 = OpVariable %_ptr_Function_int Function
         %66 = OpVariable %_ptr_Function_v2int Function
         %87 = OpVariable %_ptr_Function_v3int Function
        %106 = OpVariable %_ptr_Function_v4int Function
        %118 = OpVariable %_ptr_Function_v4float Function
         %25 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %29 = OpLoad %v4float %25
         %30 = OpVectorShuffle %v4float %29 %29 1 1 1 1
         %32 = OpVectorTimesScalar %v4float %30 %float_6
               OpStore %value %32
         %42 = OpCompositeExtract %float %32 0
         %43 = OpAccessChain %_ptr_Function_int %exp %int_0
         %41 = OpExtInst %float %1 Frexp %42 %45
         %46 = OpLoad %int %45
               OpStore %43 %46
         %47 = OpAccessChain %_ptr_Function_float %result %int_0
               OpStore %47 %41
         %50 = OpLoad %v4float %result
         %51 = OpCompositeExtract %float %50 0
         %53 = OpFOrdEqual %bool %51 %float_0_75
               OpSelectionMerge %55 None
               OpBranchConditional %53 %54 %55
         %54 = OpLabel
         %56 = OpLoad %v4int %exp
         %57 = OpCompositeExtract %int %56 0
         %59 = OpIEqual %bool %57 %int_3
               OpBranch %55
         %55 = OpLabel
         %60 = OpPhi %bool %false %22 %59 %54
         %61 = OpAccessChain %_ptr_Function_bool %ok %int_0
               OpStore %61 %60
         %64 = OpLoad %v4float %value
         %65 = OpVectorShuffle %v2float %64 %64 0 1
         %63 = OpExtInst %v2float %1 Frexp %65 %66
         %69 = OpLoad %v2int %66
         %70 = OpLoad %v4int %exp
         %71 = OpVectorShuffle %v4int %70 %69 4 5 2 3
               OpStore %exp %71
         %72 = OpLoad %v4float %result
         %73 = OpVectorShuffle %v4float %72 %63 4 5 2 3
               OpStore %result %73
         %74 = OpCompositeExtract %float %73 1
         %75 = OpFOrdEqual %bool %74 %float_0_75
               OpSelectionMerge %77 None
               OpBranchConditional %75 %76 %77
         %76 = OpLabel
         %78 = OpCompositeExtract %int %71 1
         %79 = OpIEqual %bool %78 %int_3
               OpBranch %77
         %77 = OpLabel
         %80 = OpPhi %bool %false %55 %79 %76
         %81 = OpAccessChain %_ptr_Function_bool %ok %int_1
               OpStore %81 %80
         %84 = OpLoad %v4float %value
         %85 = OpVectorShuffle %v3float %84 %84 0 1 2
         %83 = OpExtInst %v3float %1 Frexp %85 %87
         %90 = OpLoad %v3int %87
         %91 = OpLoad %v4int %exp
         %92 = OpVectorShuffle %v4int %91 %90 4 5 6 3
               OpStore %exp %92
         %93 = OpLoad %v4float %result
         %94 = OpVectorShuffle %v4float %93 %83 4 5 6 3
               OpStore %result %94
         %95 = OpCompositeExtract %float %94 2
         %96 = OpFOrdEqual %bool %95 %float_0_75
               OpSelectionMerge %98 None
               OpBranchConditional %96 %97 %98
         %97 = OpLabel
         %99 = OpCompositeExtract %int %92 2
        %100 = OpIEqual %bool %99 %int_3
               OpBranch %98
         %98 = OpLabel
        %101 = OpPhi %bool %false %77 %100 %97
        %102 = OpAccessChain %_ptr_Function_bool %ok %int_2
               OpStore %102 %101
        %105 = OpLoad %v4float %value
        %104 = OpExtInst %v4float %1 Frexp %105 %106
        %107 = OpLoad %v4int %106
               OpStore %exp %107
               OpStore %result %104
        %108 = OpCompositeExtract %float %104 3
        %109 = OpFOrdEqual %bool %108 %float_0_75
               OpSelectionMerge %111 None
               OpBranchConditional %109 %110 %111
        %110 = OpLabel
        %112 = OpCompositeExtract %int %107 3
        %113 = OpIEqual %bool %112 %int_3
               OpBranch %111
        %111 = OpLabel
        %114 = OpPhi %bool %false %98 %113 %110
        %115 = OpAccessChain %_ptr_Function_bool %ok %int_3
               OpStore %115 %114
        %117 = OpLoad %v4bool %ok
        %116 = OpAll %bool %117
               OpSelectionMerge %121 None
               OpBranchConditional %116 %119 %120
        %119 = OpLabel
        %122 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %123 = OpLoad %v4float %122
               OpStore %118 %123
               OpBranch %121
        %120 = OpLabel
        %124 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %125 = OpLoad %v4float %124
               OpStore %118 %125
               OpBranch %121
        %121 = OpLabel
        %126 = OpLoad %v4float %118
               OpReturnValue %126
               OpFunctionEnd
