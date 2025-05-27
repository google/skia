               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
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
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %88 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%mat4v4float = OpTypeMatrix %v4float 4
%_UniformBuffer = OpTypeStruct %mat4v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %13 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %17 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %21 = OpTypeFunction %v4float %_ptr_Function_v2float
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
         %43 = OpConstantComposite %v4float %float_5 %float_17 %float_38 %float_70
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
    %v3float = OpTypeVector %float 3
       %true = OpConstantTrue %bool
      %int_2 = OpConstant %int 2
%_entrypoint_v = OpFunction %void None %13
         %14 = OpLabel
         %18 = OpVariable %_ptr_Function_v2float Function
               OpStore %18 %17
         %20 = OpFunctionCall %v4float %main %18
               OpStore %sk_FragColor %20
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %21
         %22 = OpFunctionParameter %_ptr_Function_v2float
         %23 = OpLabel
     %inputA = OpVariable %_ptr_Function_v4float Function
     %inputB = OpVariable %_ptr_Function_v4float Function
   %expected = OpVariable %_ptr_Function_v4float Function
         %83 = OpVariable %_ptr_Function_v4float Function
         %26 = OpAccessChain %_ptr_Uniform_mat4v4float %7 %int_0
         %30 = OpAccessChain %_ptr_Uniform_v4float %26 %int_0
         %32 = OpLoad %v4float %30
               OpStore %inputA %32
         %34 = OpAccessChain %_ptr_Uniform_mat4v4float %7 %int_0
         %36 = OpAccessChain %_ptr_Uniform_v4float %34 %int_1
         %37 = OpLoad %v4float %36
               OpStore %inputB %37
               OpStore %expected %43
         %47 = OpCompositeExtract %float %32 0
         %48 = OpCompositeExtract %float %37 0
         %46 = OpFMul %float %47 %48
         %49 = OpFOrdEqual %bool %46 %float_5
               OpSelectionMerge %51 None
               OpBranchConditional %49 %50 %51
         %50 = OpLabel
         %53 = OpVectorShuffle %v2float %32 %32 0 1
         %54 = OpVectorShuffle %v2float %37 %37 0 1
         %52 = OpDot %float %53 %54
         %55 = OpFOrdEqual %bool %52 %float_17
               OpBranch %51
         %51 = OpLabel
         %56 = OpPhi %bool %false %23 %55 %50
               OpSelectionMerge %58 None
               OpBranchConditional %56 %57 %58
         %57 = OpLabel
         %60 = OpVectorShuffle %v3float %32 %32 0 1 2
         %62 = OpVectorShuffle %v3float %37 %37 0 1 2
         %59 = OpDot %float %60 %62
         %63 = OpFOrdEqual %bool %59 %float_38
               OpBranch %58
         %58 = OpLabel
         %64 = OpPhi %bool %false %51 %63 %57
               OpSelectionMerge %66 None
               OpBranchConditional %64 %65 %66
         %65 = OpLabel
         %67 = OpDot %float %32 %37
         %68 = OpFOrdEqual %bool %67 %float_70
               OpBranch %66
         %66 = OpLabel
         %69 = OpPhi %bool %false %58 %68 %65
               OpSelectionMerge %71 None
               OpBranchConditional %69 %70 %71
         %70 = OpLabel
               OpBranch %71
         %71 = OpLabel
         %73 = OpPhi %bool %false %66 %true %70
               OpSelectionMerge %75 None
               OpBranchConditional %73 %74 %75
         %74 = OpLabel
               OpBranch %75
         %75 = OpLabel
         %76 = OpPhi %bool %false %71 %true %74
               OpSelectionMerge %78 None
               OpBranchConditional %76 %77 %78
         %77 = OpLabel
               OpBranch %78
         %78 = OpLabel
         %79 = OpPhi %bool %false %75 %true %77
               OpSelectionMerge %81 None
               OpBranchConditional %79 %80 %81
         %80 = OpLabel
               OpBranch %81
         %81 = OpLabel
         %82 = OpPhi %bool %false %78 %true %80
               OpSelectionMerge %86 None
               OpBranchConditional %82 %84 %85
         %84 = OpLabel
         %87 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %88 = OpLoad %v4float %87
               OpStore %83 %88
               OpBranch %86
         %85 = OpLabel
         %89 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
         %91 = OpLoad %v4float %89
               OpStore %83 %91
               OpBranch %86
         %86 = OpLabel
         %92 = OpLoad %v4float %83
               OpReturnValue %92
               OpFunctionEnd
