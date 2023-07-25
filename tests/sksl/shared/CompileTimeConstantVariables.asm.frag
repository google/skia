               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %integerInput "integerInput"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %23 = OpTypeFunction %v4float %_ptr_Function_v2float
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
%float_2_1400001 = OpConstant %float 2.1400001
         %40 = OpConstantComposite %v4float %float_2_1400001 %float_2_1400001 %float_2_1400001 %float_2_1400001
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
    %float_1 = OpConstant %float 1
%float_0_200000003 = OpConstant %float 0.200000003
         %55 = OpConstantComposite %v4float %float_1 %float_0_200000003 %float_2_1400001 %float_1
%float_3_1400001 = OpConstant %float 3.1400001
         %65 = OpConstantComposite %v4float %float_3_1400001 %float_3_1400001 %float_3_1400001 %float_3_1400001
         %74 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
         %75 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
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
%integerInput = OpVariable %_ptr_Function_int Function
         %29 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %32 = OpLoad %v4float %29
         %33 = OpCompositeExtract %float %32 1
         %34 = OpConvertFToS %int %33
               OpStore %integerInput %34
         %35 = OpIEqual %bool %34 %int_0
               OpSelectionMerge %38 None
               OpBranchConditional %35 %36 %37
         %36 = OpLabel
               OpReturnValue %40
         %37 = OpLabel
         %42 = OpIEqual %bool %34 %int_1
               OpSelectionMerge %45 None
               OpBranchConditional %42 %43 %44
         %43 = OpLabel
         %46 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %47 = OpLoad %v4float %46
               OpReturnValue %47
         %44 = OpLabel
         %49 = OpIEqual %bool %34 %int_2
               OpSelectionMerge %52 None
               OpBranchConditional %49 %50 %51
         %50 = OpLabel
               OpReturnValue %55
         %51 = OpLabel
         %57 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %58 = OpLoad %v4float %57
         %59 = OpCompositeExtract %float %58 0
         %60 = OpFMul %float %59 %float_3_1400001
         %61 = OpFOrdLessThan %bool %float_3_1400001 %60
               OpSelectionMerge %64 None
               OpBranchConditional %61 %62 %63
         %62 = OpLabel
               OpReturnValue %65
         %63 = OpLabel
         %66 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %67 = OpLoad %v4float %66
         %68 = OpCompositeExtract %float %67 0
         %69 = OpFMul %float %68 %float_2_1400001
         %70 = OpFOrdGreaterThanEqual %bool %float_2_1400001 %69
               OpSelectionMerge %73 None
               OpBranchConditional %70 %71 %72
         %71 = OpLabel
               OpReturnValue %74
         %72 = OpLabel
               OpReturnValue %75
         %73 = OpLabel
               OpBranch %64
         %64 = OpLabel
               OpBranch %52
         %52 = OpLabel
               OpBranch %45
         %45 = OpLabel
               OpBranch %38
         %38 = OpLabel
               OpUnreachable
               OpFunctionEnd
