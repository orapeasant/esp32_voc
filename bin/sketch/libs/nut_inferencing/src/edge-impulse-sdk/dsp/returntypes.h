#line 1 "/home/ubuntu/proj/esp32_voc/esp32_voc/libs/nut_inferencing/src/edge-impulse-sdk/dsp/returntypes.h"
#ifndef _EIDSP_RETURN_TYPES_H_
#define _EIDSP_RETURN_TYPES_H_

#include <stdint.h>

// outside of namespace for backwards compat
typedef enum {
    EI_IMPULSE_OK = 0,
    EI_IMPULSE_ERROR_SHAPES_DONT_MATCH = -1,
    EI_IMPULSE_CANCELED = -2,
    EI_IMPULSE_TFLITE_ERROR = -3,
    EI_IMPULSE_DSP_ERROR = -5,
    EI_IMPULSE_TFLITE_ARENA_ALLOC_FAILED = -6,
    EI_IMPULSE_CUBEAI_ERROR = -7,
    EI_IMPULSE_ALLOC_FAILED = -8,
    EI_IMPULSE_ONLY_SUPPORTED_FOR_IMAGES = -9,
    EI_IMPULSE_UNSUPPORTED_INFERENCING_ENGINE = -10,
    EI_IMPULSE_OUT_OF_MEMORY = -11,
    EI_IMPULSE_INPUT_TENSOR_WAS_NULL = -13,
    EI_IMPULSE_OUTPUT_TENSOR_WAS_NULL = -14,
    EI_IMPULSE_SCORE_TENSOR_WAS_NULL = -15,
    EI_IMPULSE_LABEL_TENSOR_WAS_NULL = -16,
    EI_IMPULSE_TENSORRT_INIT_FAILED = -17,
    EI_IMPULSE_DRPAI_INIT_FAILED = -18,
    EI_IMPULSE_DRPAI_RUNTIME_FAILED = -19,
    EI_IMPULSE_DEPRECATED_MODEL = -20,
    EI_IMPULSE_LAST_LAYER_NOT_AVAILABLE = -21,
    EI_IMPULSE_INFERENCE_ERROR = -22,
    EI_IMPULSE_AKIDA_ERROR = -23,
    EI_IMPULSE_INVALID_SIZE = -24,
    EI_IMPULSE_ONNX_ERROR = -25,
    EI_IMPULSE_MEMRYX_ERROR = -26,
} EI_IMPULSE_ERROR;

#endif // _EIDSP_RETURN_TYPES_H_