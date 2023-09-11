               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "testMatrix2x2"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %inputVal "inputVal"
               OpName %expectedB "expectedB"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 ColMajor
               OpMemberDecorate %_UniformBuffer 0 MatrixStride 16
               OpMemberDecorate %_UniformBuffer 1 Offset 32
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 48
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %91 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
    %v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_UniformBuffer = OpTypeStruct %mat2v2float %v4float %v4float
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
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
   %float_n1 = OpConstant %float -1
         %41 = OpConstantComposite %v4float %float_1 %float_1 %float_n1 %float_n1
       %uint = OpTypeInt 32 0
     %v4uint = OpTypeVector %uint 4
%_ptr_Function_v4uint = OpTypePointer Function %v4uint
%uint_1065353216 = OpConstant %uint 1065353216
%uint_1073741824 = OpConstant %uint 1073741824
%uint_3225419776 = OpConstant %uint 3225419776
%uint_3229614080 = OpConstant %uint 3229614080
         %51 = OpConstantComposite %v4uint %uint_1065353216 %uint_1073741824 %uint_3225419776 %uint_3229614080
      %false = OpConstantFalse %bool
     %v2uint = OpTypeVector %uint 2
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
     %v3uint = OpTypeVector %uint 3
     %v3bool = OpTypeVector %bool 3
     %v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
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
   %inputVal = OpVariable %_ptr_Function_v4float Function
  %expectedB = OpVariable %_ptr_Function_v4uint Function
         %84 = OpVariable %_ptr_Function_v4float Function
         %29 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_0
         %33 = OpLoad %mat2v2float %29
         %34 = OpCompositeExtract %float %33 0 0
         %35 = OpCompositeExtract %float %33 0 1
         %36 = OpCompositeExtract %float %33 1 0
         %37 = OpCompositeExtract %float %33 1 1
         %38 = OpCompositeConstruct %v4float %34 %35 %36 %37
         %42 = OpFMul %v4float %38 %41
               OpStore %inputVal %42
               OpStore %expectedB %51
         %53 = OpCompositeExtract %float %42 0
         %54 = OpBitcast %float %uint_1065353216
         %55 = OpFOrdEqual %bool %53 %54
               OpSelectionMerge %57 None
               OpBranchConditional %55 %56 %57
         %56 = OpLabel
         %58 = OpVectorShuffle %v2float %42 %42 0 1
         %60 = OpVectorShuffle %v2uint %51 %51 0 1
         %59 = OpBitcast %v2float %60
         %62 = OpFOrdEqual %v2bool %58 %59
         %64 = OpAll %bool %62
               OpBranch %57
         %57 = OpLabel
         %65 = OpPhi %bool %false %26 %64 %56
               OpSelectionMerge %67 None
               OpBranchConditional %65 %66 %67
         %66 = OpLabel
         %68 = OpVectorShuffle %v3float %42 %42 0 1 2
         %71 = OpVectorShuffle %v3uint %51 %51 0 1 2
         %70 = OpBitcast %v3float %71
         %73 = OpFOrdEqual %v3bool %68 %70
         %75 = OpAll %bool %73
               OpBranch %67
         %67 = OpLabel
         %76 = OpPhi %bool %false %57 %75 %66
               OpSelectionMerge %78 None
               OpBranchConditional %76 %77 %78
         %77 = OpLabel
         %79 = OpBitcast %v4float %51
         %80 = OpFOrdEqual %v4bool %42 %79
         %82 = OpAll %bool %80
               OpBranch %78
         %78 = OpLabel
         %83 = OpPhi %bool %false %67 %82 %77
               OpSelectionMerge %87 None
               OpBranchConditional %83 %85 %86
         %85 = OpLabel
         %88 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %91 = OpLoad %v4float %88
               OpStore %84 %91
               OpBranch %87
         %86 = OpLabel
         %92 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
         %94 = OpLoad %v4float %92
               OpStore %84 %94
               OpBranch %87
         %87 = OpLabel
         %95 = OpLoad %v4float %84
               OpReturnValue %95
               OpFunctionEnd
