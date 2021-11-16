// Tencent is pleased to support the open source community by making TNN available.
//
// Copyright (C) 2020 THL A29 Limited, a Tencent company. All rights reserved.
//
// Licensed under the BSD 3-Clause License (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at
//
// https://opensource.org/licenses/BSD-3-Clause
//
// Unless required by applicable law or agreed to in writing, software distributed
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.

// author: sanerzheng@tencent.com

#include "tnn/train/grad/layer_grad.h"
#include "tnn/train/operations/op_builder.h"

namespace TNN_NS {
namespace train {
DECLARE_LAYER_GRAD(Relu, LAYER_RELU);

Status ReluLayerGrad::OnGrad(const BaseLayer *layer, TrainContext &context) {
    auto inputs  = layer->input_blobs_;
    auto outputs = layer->output_blobs_;
    if (inputs.size() != 1 || outputs.size() != 1) {
        return Status(TNN_TRAIN_ERROR, "input size or output size not match in ReluLayerGrad");
    }
    auto input0_dims = inputs[0]->GetBlobDesc().dims;
    auto output_dims = outputs[0]->GetBlobDesc().dims;
    if (!DimsVectorUtils::Equal(input0_dims, output_dims)) {
        return Status(TNN_TRAIN_ERROR, "input dims and output dims not match in ReluLayerGrad");
    }
    auto input0_data_type = inputs[0]->GetBlobDesc().data_type;
    auto output_data_type = outputs[0]->GetBlobDesc().data_type;
    if (input0_data_type != output_data_type) {
        return Status(TNN_TRAIN_ERROR, "input datatype and output datatype not match in ReluLayerGrad");
    }
    if (output_data_type != DATA_TYPE_BFP16 && output_data_type != DATA_TYPE_FLOAT) {
        return Status(TNN_TRAIN_ERROR, "output datatype not match in ReluLayerGrad");
    }

    auto iter_output = context.backward_grads_blob.find(outputs[0]);
    if (iter_output == context.backward_grads_blob.end()) {
        return Status(TNN_TRAIN_ERROR, "BinaryCrossEntropyLayerGrad output grad not find");
    }
    auto grad0 = _Mul(_RSign(ParamWrapper(inputs[0]), context), ParamWrapper(iter_output->second), context);
    if (!grad0.IsRawbufferSharedPtr()) {
        return Status(TNN_TRAIN_ERROR, "Calcute ReluLayerGrad error");
    }
    // PrintFloatBuffer(grad0.GetRawbufferSharedPtr().get());
    UpdateGradValue(inputs[0], grad0.GetRawbufferSharedPtr(), context);
    return Status(TNN_OK);
}
REGISTER_LAYER_GRAD(Relu, LAYER_RELU);

} // namespace train
} // namespace TNN_NS