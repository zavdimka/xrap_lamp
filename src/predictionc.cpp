#include "predictionc.h"
#include <iostream>

void Predictor::load_model(const std::string &file_name){
#ifdef USE_TORCH
    model = torch::jit::load(file_name);
#endif
}

std::vector<float> Predictor::process(std::vector<float> &wave){
#ifdef USE_TORCH
    auto options1 = torch::TensorOptions().dtype(torch::kFloat32);
    torch::Tensor t1 = torch::from_blob(wave.data(), {1, 1,(int)wave.size()}, options1);
    torch::Tensor pred = model.forward({t1}).toTensor();
    torch::Tensor res = torch::nn::functional::softmax(\
        pred.squeeze(), torch::nn::functional::SoftmaxFuncOptions(-1));
    std::vector<float> result(res.sizes()[0]);
    for(int i = 0; i < (int)result.size(); i++)
        result[i] = res[i].item<float>();
#else 
    std::vector<float> result(3);
#endif
    return result;
}   
