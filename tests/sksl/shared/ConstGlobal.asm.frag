               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %MATRIXFIVE "MATRIXFIVE"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %verify_const_globals_biih44 "verify_const_globals_biih44"
               OpName %main "main"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %MATRIXFIVE RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %18 Binding 0
               OpDecorate %18 DescriptorSet 0
               OpDecorate %51 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Private_mat4v4float = OpTypePointer Private %mat4v4float
 %MATRIXFIVE = OpVariable %_ptr_Private_mat4v4float Private
    %float_5 = OpConstant %float 5
    %float_0 = OpConstant %float 0
         %13 = OpConstantComposite %v4float %float_5 %float_0 %float_0 %float_0
         %14 = OpConstantComposite %v4float %float_0 %float_5 %float_0 %float_0
         %15 = OpConstantComposite %v4float %float_0 %float_0 %float_5 %float_0
         %16 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_5
         %17 = OpConstantComposite %mat4v4float %13 %14 %15 %16
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %18 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %23 = OpTypeFunction %void
    %v2float = OpTypeVector %float 2
         %26 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
         %34 = OpTypeFunction %bool %_ptr_Function_int %_ptr_Function_int %_ptr_Function_mat4v4float
      %false = OpConstantFalse %bool
      %int_7 = OpConstant %int 7
     %int_10 = OpConstant %int 10
     %v4bool = OpTypeVector %bool 4
         %69 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %23
         %24 = OpLabel
         %27 = OpVariable %_ptr_Function_v2float Function
               OpStore %27 %26
         %29 = OpFunctionCall %v4float %main %27
               OpStore %sk_FragColor %29
               OpReturn
               OpFunctionEnd
%verify_const_globals_biih44 = OpFunction %bool None %34
         %35 = OpFunctionParameter %_ptr_Function_int
         %36 = OpFunctionParameter %_ptr_Function_int
         %37 = OpFunctionParameter %_ptr_Function_mat4v4float
         %38 = OpLabel
         %40 = OpLoad %int %35
         %42 = OpIEqual %bool %40 %int_7
               OpSelectionMerge %44 None
               OpBranchConditional %42 %43 %44
         %43 = OpLabel
         %45 = OpLoad %int %36
         %47 = OpIEqual %bool %45 %int_10
               OpBranch %44
         %44 = OpLabel
         %48 = OpPhi %bool %false %38 %47 %43
               OpSelectionMerge %50 None
               OpBranchConditional %48 %49 %50
         %49 = OpLabel
         %51 = OpLoad %mat4v4float %37
         %53 = OpCompositeExtract %v4float %51 0
         %54 = OpFOrdEqual %v4bool %53 %13
         %55 = OpAll %bool %54
         %56 = OpCompositeExtract %v4float %51 1
         %57 = OpFOrdEqual %v4bool %56 %14
         %58 = OpAll %bool %57
         %59 = OpLogicalAnd %bool %55 %58
         %60 = OpCompositeExtract %v4float %51 2
         %61 = OpFOrdEqual %v4bool %60 %15
         %62 = OpAll %bool %61
         %63 = OpLogicalAnd %bool %59 %62
         %64 = OpCompositeExtract %v4float %51 3
         %65 = OpFOrdEqual %v4bool %64 %16
         %66 = OpAll %bool %65
         %67 = OpLogicalAnd %bool %63 %66
               OpBranch %50
         %50 = OpLabel
         %68 = OpPhi %bool %false %44 %67 %49
               OpReturnValue %68
               OpFunctionEnd
       %main = OpFunction %v4float None %69
         %70 = OpFunctionParameter %_ptr_Function_v2float
         %71 = OpLabel
         %72 = OpVariable %_ptr_Function_int Function
         %73 = OpVariable %_ptr_Function_int Function
         %74 = OpVariable %_ptr_Function_mat4v4float Function
         %76 = OpVariable %_ptr_Function_v4float Function
               OpStore %MATRIXFIVE %17
               OpStore %72 %int_7
               OpStore %73 %int_10
               OpStore %74 %17
         %75 = OpFunctionCall %bool %verify_const_globals_biih44 %72 %73 %74
               OpSelectionMerge %80 None
               OpBranchConditional %75 %78 %79
         %78 = OpLabel
         %81 = OpAccessChain %_ptr_Uniform_v4float %18 %int_0
         %84 = OpLoad %v4float %81
               OpStore %76 %84
               OpBranch %80
         %79 = OpLabel
         %85 = OpAccessChain %_ptr_Uniform_v4float %18 %int_1
         %87 = OpLoad %v4float %85
               OpStore %76 %87
               OpBranch %80
         %80 = OpLabel
         %88 = OpLoad %v4float %76
               OpReturnValue %88
               OpFunctionEnd
