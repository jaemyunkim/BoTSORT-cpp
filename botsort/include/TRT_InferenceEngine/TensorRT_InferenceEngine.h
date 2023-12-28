#pragma once

#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>
#include <memory>
#include <numeric>
#include <string>
#include <vector>

#include <NvInfer.h>
#include <NvOnnxParser.h>
#include <unistd.h>

#include <boost/filesystem.hpp>
#include <opencv2/opencv.hpp>

#include "TRT_Logger.h"

namespace inference_backend
{
using ModelPredictions = Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic>;

static auto StreamDeleter = [](cudaStream_t *ptr) {
    if (ptr)
    {
        cudaStreamDestroy(*ptr);
        delete ptr;
    }
};


struct TRTDestroyer
{
    template<typename T>
    void operator()(T *obj) const
    {
        if (obj)
        {
            obj->destroy();
        }
    }
};


template<typename T>
using TRTUniquePtr = std::unique_ptr<T, TRTDestroyer>;


template<typename T>
TRTUniquePtr<T> makeUnique(T *t)
{
    return TRTUniquePtr<T>{t};
}

template<typename T>
inline TRTUniquePtr<T> infer_object(T *obj)
{
    if (!obj)
    {
        throw std::runtime_error("Failed to create object");
    }
    return TRTUniquePtr<T>(obj);
}


struct TRTOptimizerParams
{
    TRTOptimizerParams() = default;

    int batch_size = 1;
    size_t max_workspace_size = 1 << 30;
    bool fp16 = true;
    bool int8 = false;
    bool tf32 = false;
    bool int8_calibrator = false;

    std::string input_layer_name;
    nvinfer1::Dims4 input_dims;

    std::vector<std::string> output_layer_names;

    std::string toStr()
    {
        // TODO
    }
};


class TensorRTInferenceEngine
{
private:
    nvinfer1::ILogger::Severity _logSeverity =
            nvinfer1::ILogger::Severity::kWARNING;
    TRTOptimizerParams _optimization_params;
    TRTUniquePtr<nvinfer1::ICudaEngine> _engine{nullptr};
    TRTUniquePtr<nvinfer1::IExecutionContext> _context{nullptr};
    std::unique_ptr<TRTLogger> _logger{nullptr};

    cudaStream_t _cuda_stream;

    std::vector<void *> _buffers;
    std::vector<nvinfer1::Dims> _input_dims;
    std::vector<nvinfer1::Dims> _output_dims;
    std::vector<std::string> _output_layer_names;

    int _input_idx = 0;
    std::vector<int> _output_idx;

public:
    TensorRTInferenceEngine(TRTOptimizerParams &optimization_params,
                            u_int8_t logging_level);
    ~TensorRTInferenceEngine();

    bool load_model(const std::string &onnx_model_path);
    ModelPredictions forward(const cv::Mat &input_image);


private:
    // Const methods
    std::string get_engine_path(const std::string &onnx_model_path) const;
    bool file_exists(const std::string &name) const;


    // Non-const methods
    void _set_optimization_params(const TRTOptimizerParams &params);
    void _init_TRT_logger(u_int8_t logging_level);

    void _build_engine(const std::string &onnx_model_path);
    bool _deserialize_engine(const std::string &engine_path);

    void print_engine_info();
    void allocate_buffers();
};
}// namespace inference_backend