               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "testMatrix4x4"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %inputA "inputA"
               OpName %inputB "inputB"
               OpName %expected "expected"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 ColMajor
               OpMemberDecorate %_UniformBuffer 0 MatrixStride 16
               OpMemberDecorate %_UniformBuffer 1 Offset 64
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 80
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %90 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%mat4v4float = OpTypeMatrix %v4float 4
%_UniformBuffer = OpTypeStruct %mat4v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %24 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_1 = OpConstant %int 1
    %float_5 = OpConstant %float 5
   %float_17 = OpConstant %float 17
   %float_38 = OpConstant %float 38
   %float_70 = OpConstant %float 70
         %46 = OpConstantComposite %v4float %float_5 %float_17 %float_38 %float_70
      %false = OpConstantFalse %bool
    %v3float = OpTypeVector %float 3
       %true = OpConstantTrue %bool
      %int_2 = OpConstant %int 2
%_entrypoint_v = OpFunction %void None %16
         %17 = OpLabel
         %21 = OpVariable %_ptr_Function_v2float Function
               OpStore %21 %20
         %23 = OpFunctionCall %v4float %main %21
               OpStore %sk_FragColor %23
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %24
         %25 = OpFunctionParameter %_ptr_Function_v2float
         %26 = OpLabel
     %inputA = OpVariable %_ptr_Function_v4float Function
     %inputB = OpVariable %_ptr_Function_v4float Function
   %expected = OpVariable %_ptr_Function_v4float Function
         %85 = OpVariable %_ptr_Function_v4float Function
         %29 = OpAccessChain %_ptr_Uniform_mat4v4float %10 %int_0
         %33 = OpAccessChain %_ptr_Uniform_v4float %29 %int_0
         %35 = OpLoad %v4float %33
               OpStore %inputA %35
         %37 = OpAccessChain %_ptr_Uniform_mat4v4float %10 %int_0
         %39 = OpAccessChain %_ptr_Uniform_v4float %37 %int_1
         %40 = OpLoad %v4float %39
               OpStore %inputB %40
               OpStore %expected %46
         %49 = OpCompositeExtract %float %35 0
         %50 = OpCompositeExtract %float %40 0
         %48 = OpFMul %float %49 %50
         %51 = OpFOrdEqual %bool %48 %float_5
               OpSelectionMerge %53 None
               OpBranchConditional %51 %52 %53
         %52 = OpLabel
         %55 = OpVectorShuffle %v2float %35 %35 0 1
         %56 = OpVectorShuffle %v2float %40 %40 0 1
         %54 = OpDot %float %55 %56
         %57 = OpFOrdEqual %bool %54 %float_17
               OpBranch %53
         %53 = OpLabel
         %58 = OpPhi %bool %false %26 %57 %52
               OpSelectionMerge %60 None
               OpBranchConditional %58 %59 %60
         %59 = OpLabel
         %62 = OpVectorShuffle %v3float %35 %35 0 1 2
         %64 = OpVectorShuffle %v3float %40 %40 0 1 2
         %61 = OpDot %float %62 %64
         %65 = OpFOrdEqual %bool %61 %float_38
               OpBranch %60
         %60 = OpLabel
         %66 = OpPhi %bool %false %53 %65 %59
               OpSelectionMerge %68 None
               OpBranchConditional %66 %67 %68
         %67 = OpLabel
         %69 = OpDot %float %35 %40
         %70 = OpFOrdEqual %bool %69 %float_70
               OpBranch %68
         %68 = OpLabel
         %71 = OpPhi %bool %false %60 %70 %67
               OpSelectionMerge %73 None
               OpBranchConditional %71 %72 %73
         %72 = OpLabel
               OpBranch %73
         %73 = OpLabel
         %75 = OpPhi %bool %false %68 %true %72
               OpSelectionMerge %77 None
               OpBranchConditional %75 %76 %77
         %76 = OpLabel
               OpBranch %77
         %77 = OpLabel
         %78 = OpPhi %bool %false %73 %true %76
               OpSelectionMerge %80 None
               OpBranchConditional %78 %79 %80
         %79 = OpLabel
               OpBranch %80
         %80 = OpLabel
         %81 = OpPhi %bool %false %77 %true %79
               OpSelectionMerge %83 None
               OpBranchConditional %81 %82 %83
         %82 = OpLabel
               OpBranch %83
         %83 = OpLabel
         %84 = OpPhi %bool %false %80 %true %82
               OpSelectionMerge %88 None
               OpBranchConditional %84 %86 %87
         %86 = OpLabel
         %89 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %90 = OpLoad %v4float %89
               OpStore %85 %90
               OpBranch %88
         %87 = OpLabel
         %91 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
         %93 = OpLoad %v4float %91
               OpStore %85 %93
               OpBranch %88
         %88 = OpLabel
         %94 = OpLoad %v4float %85
               OpReturnValue %94
               OpFunctionEnd
