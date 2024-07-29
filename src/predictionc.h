#pragma once

#include <string>
#include <vector>

#undef USE_TORCH //use for fast test, long compiling time

#ifdef USE_TORCH

    #undef slots

    #include <torch/torch.h>
    #include <torch/script.h>

    #define slots Q_SLOTS

#endif

class Predictor{
    public:
    Predictor(){};

    void load_model(const std::string &file_name);
    std::vector<float> process(std::vector<float> &wave);

    private:
#ifdef USE_TORCH
    torch::jit::script::Module model;
#endif
};
