               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %testPushConstants "testPushConstants"    ; id %11
               OpMemberName %testPushConstants 0 "pushConstantArray"
               OpName %testUniforms "testUniforms"  ; id %15
               OpMemberName %testUniforms 0 "uniformArray"
               OpName %testStorageBuffer "testStorageBuffer"    ; id %18
               OpMemberName %testStorageBuffer 0 "ssboArray"
               OpName %sk_FragColor "sk_FragColor"  ; id %20
               OpName %main "main"                  ; id %6
               OpName %localArray "localArray"      ; id %26

               ; Annotations
               OpDecorate %_arr_float_int_2 ArrayStride 4
               OpMemberDecorate %testPushConstants 0 Offset 0
               OpDecorate %testPushConstants Block
               OpDecorate %_arr_float_int_2_0 ArrayStride 16
               OpMemberDecorate %testUniforms 0 Offset 0
               OpDecorate %testUniforms Block
               OpDecorate %13 Binding 0
               OpDecorate %13 DescriptorSet 0
               OpMemberDecorate %testStorageBuffer 0 Offset 0
               OpDecorate %testStorageBuffer BufferBlock
               OpDecorate %17 Binding 1
               OpDecorate %17 DescriptorSet 0
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %80 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
      %int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2       ; ArrayStride 4
%testPushConstants = OpTypeStruct %_arr_float_int_2     ; Block
%_ptr_PushConstant_testPushConstants = OpTypePointer PushConstant %testPushConstants
          %7 = OpVariable %_ptr_PushConstant_testPushConstants PushConstant
%_arr_float_int_2_0 = OpTypeArray %float %int_2     ; ArrayStride 16
%testUniforms = OpTypeStruct %_arr_float_int_2_0    ; Block
%_ptr_Uniform_testUniforms = OpTypePointer Uniform %testUniforms
         %13 = OpVariable %_ptr_Uniform_testUniforms Uniform    ; Binding 0, DescriptorSet 0
%testStorageBuffer = OpTypeStruct %_arr_float_int_2             ; BufferBlock
%_ptr_Uniform_testStorageBuffer = OpTypePointer Uniform %testStorageBuffer
         %17 = OpVariable %_ptr_Uniform_testStorageBuffer Uniform   ; Binding 1, DescriptorSet 0
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
       %void = OpTypeVoid
         %24 = OpTypeFunction %void
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
         %77 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
    %float_0 = OpConstant %float 0
         %79 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0


               ; Function main
       %main = OpFunction %void None %24

         %25 = OpLabel
 %localArray =   OpVariable %_ptr_Function__arr_float_int_2_0 Function
         %72 =   OpVariable %_ptr_Function_v4float Function
         %30 =   OpCompositeConstruct %_arr_float_int_2_0 %float_1 %float_2
                 OpStore %localArray %30
         %34 =   OpAccessChain %_ptr_Uniform__arr_float_int_2_0 %13 %int_0
         %36 =   OpLoad %_arr_float_int_2_0 %34
         %37 =   OpCompositeExtract %float %36 0
         %38 =   OpFOrdEqual %bool %float_1 %37
         %39 =   OpCompositeExtract %float %36 1
         %40 =   OpFOrdEqual %bool %float_2 %39
         %41 =   OpLogicalAnd %bool %40 %38
                 OpSelectionMerge %43 None
                 OpBranchConditional %41 %42 %43

         %42 =     OpLabel
         %44 =       OpAccessChain %_ptr_Uniform__arr_float_int_2_0 %13 %int_0
         %45 =       OpLoad %_arr_float_int_2_0 %44
         %46 =       OpAccessChain %_ptr_PushConstant__arr_float_int_2 %7 %int_0
         %48 =       OpLoad %_arr_float_int_2 %46
         %49 =       OpCompositeExtract %float %45 0
         %50 =       OpCompositeExtract %float %48 0
         %51 =       OpFOrdEqual %bool %49 %50
         %52 =       OpCompositeExtract %float %45 1
         %53 =       OpCompositeExtract %float %48 1
         %54 =       OpFOrdEqual %bool %52 %53
         %55 =       OpLogicalAnd %bool %54 %51
                     OpBranch %43

         %43 = OpLabel
         %56 =   OpPhi %bool %false %25 %55 %42
                 OpSelectionMerge %58 None
                 OpBranchConditional %56 %57 %58

         %57 =     OpLabel
         %59 =       OpAccessChain %_ptr_Uniform__arr_float_int_2_0 %13 %int_0
         %60 =       OpLoad %_arr_float_int_2_0 %59
         %61 =       OpAccessChain %_ptr_Uniform__arr_float_int_2 %17 %int_0
         %63 =       OpLoad %_arr_float_int_2 %61
         %64 =       OpCompositeExtract %float %60 0
         %65 =       OpCompositeExtract %float %63 0
         %66 =       OpFOrdEqual %bool %64 %65
         %67 =       OpCompositeExtract %float %60 1
         %68 =       OpCompositeExtract %float %63 1
         %69 =       OpFOrdEqual %bool %67 %68
         %70 =       OpLogicalAnd %bool %69 %66
                     OpBranch %58

         %58 = OpLabel
         %71 =   OpPhi %bool %false %43 %70 %57
                 OpSelectionMerge %76 None
                 OpBranchConditional %71 %74 %75

         %74 =     OpLabel
                     OpStore %72 %77
                     OpBranch %76

         %75 =     OpLabel
                     OpStore %72 %79
                     OpBranch %76

         %76 = OpLabel
         %80 =   OpLoad %v4float %72                ; RelaxedPrecision
                 OpStore %sk_FragColor %80
                 OpReturn
               OpFunctionEnd
