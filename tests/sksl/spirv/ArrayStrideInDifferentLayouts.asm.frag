               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %testPushConstants "testPushConstants"
               OpMemberName %testPushConstants 0 "pushConstantArray"
               OpName %testUniforms "testUniforms"
               OpMemberName %testUniforms 0 "uniformArray"
               OpName %testStorageBuffer "testStorageBuffer"
               OpMemberName %testStorageBuffer 0 "ssboArray"
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
               OpMemberDecorate %testStorageBuffer 0 Offset 0
               OpDecorate %testStorageBuffer BufferBlock
               OpDecorate %14 Binding 1
               OpDecorate %14 DescriptorSet 0
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %77 RelaxedPrecision
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
%testStorageBuffer = OpTypeStruct %_arr_float_int_2
%_ptr_Uniform_testStorageBuffer = OpTypePointer Uniform %testStorageBuffer
         %14 = OpVariable %_ptr_Uniform_testStorageBuffer Uniform
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
       %void = OpTypeVoid
         %21 = OpTypeFunction %void
%_ptr_Function__arr_float_int_2_0 = OpTypePointer Function %_arr_float_int_2_0
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
      %int_0 = OpConstant %int 0
%_ptr_Uniform__arr_float_int_2_0 = OpTypePointer Uniform %_arr_float_int_2_0
%_ptr_PushConstant__arr_float_int_2 = OpTypePointer PushConstant %_arr_float_int_2
%_ptr_Uniform__arr_float_int_2 = OpTypePointer Uniform %_arr_float_int_2
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %74 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
    %float_0 = OpConstant %float 0
         %76 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
       %main = OpFunction %void None %21
         %22 = OpLabel
 %localArray = OpVariable %_ptr_Function__arr_float_int_2_0 Function
         %69 = OpVariable %_ptr_Function_v4float Function
         %27 = OpCompositeConstruct %_arr_float_int_2_0 %float_1 %float_2
               OpStore %localArray %27
         %31 = OpAccessChain %_ptr_Uniform__arr_float_int_2_0 %10 %int_0
         %33 = OpLoad %_arr_float_int_2_0 %31
         %34 = OpCompositeExtract %float %33 0
         %35 = OpFOrdEqual %bool %float_1 %34
         %36 = OpCompositeExtract %float %33 1
         %37 = OpFOrdEqual %bool %float_2 %36
         %38 = OpLogicalAnd %bool %37 %35
               OpSelectionMerge %40 None
               OpBranchConditional %38 %39 %40
         %39 = OpLabel
         %41 = OpAccessChain %_ptr_Uniform__arr_float_int_2_0 %10 %int_0
         %42 = OpLoad %_arr_float_int_2_0 %41
         %43 = OpAccessChain %_ptr_PushConstant__arr_float_int_2 %3 %int_0
         %45 = OpLoad %_arr_float_int_2 %43
         %46 = OpCompositeExtract %float %42 0
         %47 = OpCompositeExtract %float %45 0
         %48 = OpFOrdEqual %bool %46 %47
         %49 = OpCompositeExtract %float %42 1
         %50 = OpCompositeExtract %float %45 1
         %51 = OpFOrdEqual %bool %49 %50
         %52 = OpLogicalAnd %bool %51 %48
               OpBranch %40
         %40 = OpLabel
         %53 = OpPhi %bool %false %22 %52 %39
               OpSelectionMerge %55 None
               OpBranchConditional %53 %54 %55
         %54 = OpLabel
         %56 = OpAccessChain %_ptr_Uniform__arr_float_int_2_0 %10 %int_0
         %57 = OpLoad %_arr_float_int_2_0 %56
         %58 = OpAccessChain %_ptr_Uniform__arr_float_int_2 %14 %int_0
         %60 = OpLoad %_arr_float_int_2 %58
         %61 = OpCompositeExtract %float %57 0
         %62 = OpCompositeExtract %float %60 0
         %63 = OpFOrdEqual %bool %61 %62
         %64 = OpCompositeExtract %float %57 1
         %65 = OpCompositeExtract %float %60 1
         %66 = OpFOrdEqual %bool %64 %65
         %67 = OpLogicalAnd %bool %66 %63
               OpBranch %55
         %55 = OpLabel
         %68 = OpPhi %bool %false %40 %67 %54
               OpSelectionMerge %73 None
               OpBranchConditional %68 %71 %72
         %71 = OpLabel
               OpStore %69 %74
               OpBranch %73
         %72 = OpLabel
               OpStore %69 %76
               OpBranch %73
         %73 = OpLabel
         %77 = OpLoad %v4float %69
               OpStore %sk_FragColor %77
               OpReturn
               OpFunctionEnd
