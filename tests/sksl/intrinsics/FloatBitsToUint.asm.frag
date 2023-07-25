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
               OpDecorate %89 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
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
       %uint = OpTypeInt 32 0
%uint_1065353216 = OpConstant %uint 1065353216
     %v2uint = OpTypeVector %uint 2
%uint_1073741824 = OpConstant %uint 1073741824
         %55 = OpConstantComposite %v2uint %uint_1065353216 %uint_1073741824
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
     %v3uint = OpTypeVector %uint 3
%uint_3225419776 = OpConstant %uint 3225419776
         %67 = OpConstantComposite %v3uint %uint_1065353216 %uint_1073741824 %uint_3225419776
     %v3bool = OpTypeVector %bool 3
     %v4uint = OpTypeVector %uint 4
%uint_3229614080 = OpConstant %uint 3229614080
         %77 = OpConstantComposite %v4uint %uint_1065353216 %uint_1073741824 %uint_3225419776 %uint_3229614080
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
         %82 = OpVariable %_ptr_Function_v4float Function
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
         %44 = OpBitcast %uint %45
         %48 = OpIEqual %bool %44 %uint_1065353216
               OpSelectionMerge %50 None
               OpBranchConditional %48 %49 %50
         %49 = OpLabel
         %52 = OpVectorShuffle %v2float %42 %42 0 1
         %51 = OpBitcast %v2uint %52
         %56 = OpIEqual %v2bool %51 %55
         %58 = OpAll %bool %56
               OpBranch %50
         %50 = OpLabel
         %59 = OpPhi %bool %false %26 %58 %49
               OpSelectionMerge %61 None
               OpBranchConditional %59 %60 %61
         %60 = OpLabel
         %63 = OpVectorShuffle %v3float %42 %42 0 1 2
         %62 = OpBitcast %v3uint %63
         %68 = OpIEqual %v3bool %62 %67
         %70 = OpAll %bool %68
               OpBranch %61
         %61 = OpLabel
         %71 = OpPhi %bool %false %50 %70 %60
               OpSelectionMerge %73 None
               OpBranchConditional %71 %72 %73
         %72 = OpLabel
         %74 = OpBitcast %v4uint %42
         %78 = OpIEqual %v4bool %74 %77
         %80 = OpAll %bool %78
               OpBranch %73
         %73 = OpLabel
         %81 = OpPhi %bool %false %61 %80 %72
               OpSelectionMerge %85 None
               OpBranchConditional %81 %83 %84
         %83 = OpLabel
         %86 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %89 = OpLoad %v4float %86
               OpStore %82 %89
               OpBranch %85
         %84 = OpLabel
         %90 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
         %92 = OpLoad %v4float %90
               OpStore %82 %92
               OpBranch %85
         %85 = OpLabel
         %93 = OpLoad %v4float %82
               OpReturnValue %93
               OpFunctionEnd
