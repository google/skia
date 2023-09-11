               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %testPushConstants "testPushConstants"
               OpMemberName %testPushConstants 0 "pushConstantArray"
               OpName %testUniforms "testUniforms"
               OpMemberName %testUniforms 0 "uniformArray"
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %main "main"
               OpName %localArray "localArray"
               OpDecorate %_arr_float_int_2 ArrayStride 4
               OpMemberDecorate %testPushConstants 0 Offset 0
               OpDecorate %testPushConstants Block
               OpDecorate %_arr_float_int_2_0 ArrayStride 16
               OpMemberDecorate %testUniforms 0 Offset 0
               OpDecorate %testUniforms Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %61 RelaxedPrecision
      %float = OpTypeFloat 32
        %int = OpTypeInt 32 1
      %int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2
%testPushConstants = OpTypeStruct %_arr_float_int_2
%_ptr_PushConstant_testPushConstants = OpTypePointer PushConstant %testPushConstants
          %3 = OpVariable %_ptr_PushConstant_testPushConstants PushConstant
%_arr_float_int_2_0 = OpTypeArray %float %int_2
%testUniforms = OpTypeStruct %_arr_float_int_2_0
%_ptr_Uniform_testUniforms = OpTypePointer Uniform %testUniforms
         %10 = OpVariable %_ptr_Uniform_testUniforms Uniform
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
       %void = OpTypeVoid
         %21 = OpTypeFunction %void
%_ptr_Function__arr_float_int_2_0 = OpTypePointer Function %_arr_float_int_2_0
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
      %false = OpConstantFalse %bool
      %int_0 = OpConstant %int 0
%_ptr_Uniform__arr_float_int_2_0 = OpTypePointer Uniform %_arr_float_int_2_0
%_ptr_PushConstant__arr_float_int_2 = OpTypePointer PushConstant %_arr_float_int_2
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %58 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
    %float_0 = OpConstant %float 0
         %60 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
       %main = OpFunction %void None %21
         %22 = OpLabel
 %localArray = OpVariable %_ptr_Function__arr_float_int_2_0 Function
         %53 = OpVariable %_ptr_Function_v4float Function
         %27 = OpCompositeConstruct %_arr_float_int_2_0 %float_1 %float_2
               OpStore %localArray %27
         %30 = OpAccessChain %_ptr_Uniform__arr_float_int_2_0 %10 %int_0
         %32 = OpLoad %_arr_float_int_2_0 %30
         %33 = OpCompositeExtract %float %32 0
         %34 = OpFOrdEqual %bool %float_1 %33
         %35 = OpCompositeExtract %float %32 1
         %36 = OpFOrdEqual %bool %float_2 %35
         %37 = OpLogicalAnd %bool %36 %34
               OpSelectionMerge %39 None
               OpBranchConditional %37 %38 %39
         %38 = OpLabel
         %40 = OpAccessChain %_ptr_Uniform__arr_float_int_2_0 %10 %int_0
         %41 = OpLoad %_arr_float_int_2_0 %40
         %42 = OpAccessChain %_ptr_PushConstant__arr_float_int_2 %3 %int_0
         %44 = OpLoad %_arr_float_int_2 %42
         %45 = OpCompositeExtract %float %41 0
         %46 = OpCompositeExtract %float %44 0
         %47 = OpFOrdEqual %bool %45 %46
         %48 = OpCompositeExtract %float %41 1
         %49 = OpCompositeExtract %float %44 1
         %50 = OpFOrdEqual %bool %48 %49
         %51 = OpLogicalAnd %bool %50 %47
               OpBranch %39
         %39 = OpLabel
         %52 = OpPhi %bool %false %22 %51 %38
               OpSelectionMerge %57 None
               OpBranchConditional %52 %55 %56
         %55 = OpLabel
               OpStore %53 %58
               OpBranch %57
         %56 = OpLabel
               OpStore %53 %60
               OpBranch %57
         %57 = OpLabel
         %61 = OpLoad %v4float %53
               OpStore %sk_FragColor %61
               OpReturn
               OpFunctionEnd
