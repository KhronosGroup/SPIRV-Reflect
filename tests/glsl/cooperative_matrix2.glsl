#version 450
#pragma use_vulkan_memory_model
#extension GL_KHR_memory_scope_semantics : enable
#extension GL_KHR_cooperative_matrix : enable
#extension GL_NV_cooperative_matrix2 : enable
#extension GL_EXT_buffer_reference : enable

layout(local_size_x = 64) in;

layout(set = 0, binding = 0) buffer InputData {
    float x[];
} input_data;

layout(set = 0, binding = 1) buffer OutputData {
    float x[];
} output_data;

layout(buffer_reference, std430, buffer_reference_align = 4) buffer Block {
    float value;
};

float decodeFunc(const in Block block, const in uint blockCoords[2], const in uint coordInBlock[2]) {
    return block.value;
}

float reduceFunc(const in float a, const in float b) {
    return a + b;
}

float perElementFunc(const in uint row, const in uint col, const in float x, const in float y) {
    return x * y;
}

void main() {
    // Function variables of these types add OpTypePointer instructions that point at them
    tensorLayoutNV<2> tensor_layout = createTensorLayoutNV(2);
    tensor_layout = setTensorLayoutDimensionNV(tensor_layout, 16, 16);
    tensor_layout = setTensorLayoutStrideNV(tensor_layout, 16, 1);
    tensor_layout = sliceTensorLayoutNV(tensor_layout, 0, 8, 0, 8);
    tensor_layout = setTensorLayoutClampValueNV(tensor_layout, 0);

    tensorLayoutNV<2> block_layout = createTensorLayoutNV(2);
    block_layout = setTensorLayoutBlockSizeNV(block_layout, 4, 4);
    block_layout = setTensorLayoutDimensionNV(block_layout, 16, 16);

    tensorViewNV<2, true, 1, 0> tensor_view = createTensorViewNV(2, true, 1, 0);
    tensor_view = setTensorViewDimensionsNV(tensor_view, 16, 16);
    tensor_view = setTensorViewStrideNV(tensor_view, 16, 1);
    tensor_view = setTensorViewClipNV(tensor_view, 0, 4, 0, 4);

    coopmat<float, gl_ScopeSubgroup, 4, 4, gl_MatrixUseAccumulator> decoded;
    coopMatLoadTensorNV(decoded, input_data.x, 0, block_layout, decodeFunc);

    coopmat<float, gl_ScopeSubgroup, 4, 4, gl_MatrixUseAccumulator> loaded;
    coopMatLoadTensorNV(loaded, input_data.x, 0, tensor_layout, tensor_view);

    coopmat<float, gl_ScopeSubgroup, 4, 4, gl_MatrixUseA> converted = coopmat<float, gl_ScopeSubgroup, 4, 4, gl_MatrixUseA>(decoded);

    coopmat<float, gl_ScopeSubgroup, 4, 4, gl_MatrixUseB> transposed;
    coopMatTransposeNV(transposed, decoded);

    coopmat<float, gl_ScopeSubgroup, 4, 4, gl_MatrixUseAccumulator> reduced;
    coopMatReduceNV(reduced, decoded, gl_CooperativeMatrixReduceRowAndColumnNV, reduceFunc);

    coopmat<float, gl_ScopeSubgroup, 4, 4, gl_MatrixUseAccumulator> per_element;
    coopMatPerElementNV(per_element, reduced, perElementFunc, loaded);

    coopMatStoreTensorNV(per_element, output_data.x, 0, tensor_layout, tensor_view);
}
