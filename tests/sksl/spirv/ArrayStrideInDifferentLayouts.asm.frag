               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %testPushConstants "testPushConstants"
               OpMemberName %testPushConstants 0 "pushConstantArray"
               OpName %testUniforms "testUniforms"
               OpMemberName %testUniforms 0 "uniformArray"
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
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %59 RelaxedPrecision
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
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
       %void = OpTypeVoid
         %18 = OpTypeFunction %void
%_ptr_Function__arr_float_int_2_0 = OpTypePointer Function %_arr_float_int_2_0
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
      %int_0 = OpConstant %int 0
%_ptr_Uniform__arr_float_int_2_0 = OpTypePointer Uniform %_arr_float_int_2_0
%_ptr_PushConstant__arr_float_int_2 = OpTypePointer PushConstant %_arr_float_int_2
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %56 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
    %float_0 = OpConstant %float 0
         %58 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
       %main = OpFunction %void None %18
         %19 = OpLabel
 %localArray = OpVariable %_ptr_Function__arr_float_int_2_0 Function
         %51 = OpVariable %_ptr_Function_v4float Function
         %24 = OpCompositeConstruct %_arr_float_int_2_0 %float_1 %float_2
               OpStore %localArray %24
         %28 = OpAccessChain %_ptr_Uniform__arr_float_int_2_0 %10 %int_0
         %30 = OpLoad %_arr_float_int_2_0 %28
         %31 = OpCompositeExtract %float %30 0
         %32 = OpFOrdEqual %bool %float_1 %31
         %33 = OpCompositeExtract %float %30 1
         %34 = OpFOrdEqual %bool %float_2 %33
         %35 = OpLogicalAnd %bool %34 %32
               OpSelectionMerge %37 None
               OpBranchConditional %35 %36 %37
         %36 = OpLabel
         %38 = OpAccessChain %_ptr_Uniform__arr_float_int_2_0 %10 %int_0
         %39 = OpLoad %_arr_float_int_2_0 %38
         %40 = OpAccessChain %_ptr_PushConstant__arr_float_int_2 %3 %int_0
         %42 = OpLoad %_arr_float_int_2 %40
         %43 = OpCompositeExtract %float %39 0
         %44 = OpCompositeExtract %float %42 0
         %45 = OpFOrdEqual %bool %43 %44
         %46 = OpCompositeExtract %float %39 1
         %47 = OpCompositeExtract %float %42 1
         %48 = OpFOrdEqual %bool %46 %47
         %49 = OpLogicalAnd %bool %48 %45
               OpBranch %37
         %37 = OpLabel
         %50 = OpPhi %bool %false %19 %49 %36
               OpSelectionMerge %55 None
               OpBranchConditional %50 %53 %54
         %53 = OpLabel
               OpStore %51 %56
               OpBranch %55
         %54 = OpLabel
               OpStore %51 %58
               OpBranch %55
         %55 = OpLabel
         %59 = OpLoad %v4float %51
               OpStore %sk_FragColor %59
               OpReturn
               OpFunctionEnd
