               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %MATRIXFIVE "MATRIXFIVE"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %verify_const_globals_biih44 "verify_const_globals_biih44"
               OpName %main "main"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %MATRIXFIVE RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %21 Binding 0
               OpDecorate %21 DescriptorSet 0
               OpDecorate %53 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Private_mat4v4float = OpTypePointer Private %mat4v4float
 %MATRIXFIVE = OpVariable %_ptr_Private_mat4v4float Private
    %float_5 = OpConstant %float 5
    %float_0 = OpConstant %float 0
         %16 = OpConstantComposite %v4float %float_5 %float_0 %float_0 %float_0
         %17 = OpConstantComposite %v4float %float_0 %float_5 %float_0 %float_0
         %18 = OpConstantComposite %v4float %float_0 %float_0 %float_5 %float_0
         %19 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_5
         %20 = OpConstantComposite %mat4v4float %16 %17 %18 %19
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %21 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %26 = OpTypeFunction %void
    %v2float = OpTypeVector %float 2
         %29 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
         %36 = OpTypeFunction %bool %_ptr_Function_int %_ptr_Function_int %_ptr_Function_mat4v4float
      %false = OpConstantFalse %bool
      %int_7 = OpConstant %int 7
     %int_10 = OpConstant %int 10
     %v4bool = OpTypeVector %bool 4
         %71 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %26
         %27 = OpLabel
         %30 = OpVariable %_ptr_Function_v2float Function
               OpStore %30 %29
         %32 = OpFunctionCall %v4float %main %30
               OpStore %sk_FragColor %32
               OpReturn
               OpFunctionEnd
%verify_const_globals_biih44 = OpFunction %bool None %36
         %37 = OpFunctionParameter %_ptr_Function_int
         %38 = OpFunctionParameter %_ptr_Function_int
         %39 = OpFunctionParameter %_ptr_Function_mat4v4float
         %40 = OpLabel
         %42 = OpLoad %int %37
         %44 = OpIEqual %bool %42 %int_7
               OpSelectionMerge %46 None
               OpBranchConditional %44 %45 %46
         %45 = OpLabel
         %47 = OpLoad %int %38
         %49 = OpIEqual %bool %47 %int_10
               OpBranch %46
         %46 = OpLabel
         %50 = OpPhi %bool %false %40 %49 %45
               OpSelectionMerge %52 None
               OpBranchConditional %50 %51 %52
         %51 = OpLabel
         %53 = OpLoad %mat4v4float %39
         %55 = OpCompositeExtract %v4float %53 0
         %56 = OpFOrdEqual %v4bool %55 %16
         %57 = OpAll %bool %56
         %58 = OpCompositeExtract %v4float %53 1
         %59 = OpFOrdEqual %v4bool %58 %17
         %60 = OpAll %bool %59
         %61 = OpLogicalAnd %bool %57 %60
         %62 = OpCompositeExtract %v4float %53 2
         %63 = OpFOrdEqual %v4bool %62 %18
         %64 = OpAll %bool %63
         %65 = OpLogicalAnd %bool %61 %64
         %66 = OpCompositeExtract %v4float %53 3
         %67 = OpFOrdEqual %v4bool %66 %19
         %68 = OpAll %bool %67
         %69 = OpLogicalAnd %bool %65 %68
               OpBranch %52
         %52 = OpLabel
         %70 = OpPhi %bool %false %46 %69 %51
               OpReturnValue %70
               OpFunctionEnd
       %main = OpFunction %v4float None %71
         %72 = OpFunctionParameter %_ptr_Function_v2float
         %73 = OpLabel
         %74 = OpVariable %_ptr_Function_int Function
         %75 = OpVariable %_ptr_Function_int Function
         %76 = OpVariable %_ptr_Function_mat4v4float Function
         %78 = OpVariable %_ptr_Function_v4float Function
               OpStore %MATRIXFIVE %20
               OpStore %74 %int_7
               OpStore %75 %int_10
               OpStore %76 %20
         %77 = OpFunctionCall %bool %verify_const_globals_biih44 %74 %75 %76
               OpSelectionMerge %82 None
               OpBranchConditional %77 %80 %81
         %80 = OpLabel
         %83 = OpAccessChain %_ptr_Uniform_v4float %21 %int_0
         %86 = OpLoad %v4float %83
               OpStore %78 %86
               OpBranch %82
         %81 = OpLabel
         %87 = OpAccessChain %_ptr_Uniform_v4float %21 %int_1
         %89 = OpLoad %v4float %87
               OpStore %78 %89
               OpBranch %82
         %82 = OpLabel
         %90 = OpLoad %v4float %78
               OpReturnValue %90
               OpFunctionEnd
