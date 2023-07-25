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
               OpDecorate %88 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
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
      %false = OpConstantFalse %bool
%int_1065353216 = OpConstant %int 1065353216
      %v2int = OpTypeVector %int 2
%int_1073741824 = OpConstant %int 1073741824
         %54 = OpConstantComposite %v2int %int_1065353216 %int_1073741824
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
      %v3int = OpTypeVector %int 3
%int_n1069547520 = OpConstant %int -1069547520
         %66 = OpConstantComposite %v3int %int_1065353216 %int_1073741824 %int_n1069547520
     %v3bool = OpTypeVector %bool 3
      %v4int = OpTypeVector %int 4
%int_n1065353216 = OpConstant %int -1065353216
         %76 = OpConstantComposite %v4int %int_1065353216 %int_1073741824 %int_n1069547520 %int_n1065353216
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
         %81 = OpVariable %_ptr_Function_v4float Function
         %29 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_0
         %33 = OpLoad %mat2v2float %29
         %34 = OpCompositeExtract %float %33 0 0
         %35 = OpCompositeExtract %float %33 0 1
         %36 = OpCompositeExtract %float %33 1 0
         %37 = OpCompositeExtract %float %33 1 1
         %38 = OpCompositeConstruct %v4float %34 %35 %36 %37
         %42 = OpFMul %v4float %38 %41
               OpStore %inputVal %42
         %45 = OpCompositeExtract %float %42 0
         %44 = OpBitcast %int %45
         %47 = OpIEqual %bool %44 %int_1065353216
               OpSelectionMerge %49 None
               OpBranchConditional %47 %48 %49
         %48 = OpLabel
         %51 = OpVectorShuffle %v2float %42 %42 0 1
         %50 = OpBitcast %v2int %51
         %55 = OpIEqual %v2bool %50 %54
         %57 = OpAll %bool %55
               OpBranch %49
         %49 = OpLabel
         %58 = OpPhi %bool %false %26 %57 %48
               OpSelectionMerge %60 None
               OpBranchConditional %58 %59 %60
         %59 = OpLabel
         %62 = OpVectorShuffle %v3float %42 %42 0 1 2
         %61 = OpBitcast %v3int %62
         %67 = OpIEqual %v3bool %61 %66
         %69 = OpAll %bool %67
               OpBranch %60
         %60 = OpLabel
         %70 = OpPhi %bool %false %49 %69 %59
               OpSelectionMerge %72 None
               OpBranchConditional %70 %71 %72
         %71 = OpLabel
         %73 = OpBitcast %v4int %42
         %77 = OpIEqual %v4bool %73 %76
         %79 = OpAll %bool %77
               OpBranch %72
         %72 = OpLabel
         %80 = OpPhi %bool %false %60 %79 %71
               OpSelectionMerge %84 None
               OpBranchConditional %80 %82 %83
         %82 = OpLabel
         %85 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %88 = OpLoad %v4float %85
               OpStore %81 %88
               OpBranch %84
         %83 = OpLabel
         %89 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
         %91 = OpLoad %v4float %89
               OpStore %81 %91
               OpBranch %84
         %84 = OpLabel
         %92 = OpLoad %v4float %81
               OpReturnValue %92
               OpFunctionEnd
