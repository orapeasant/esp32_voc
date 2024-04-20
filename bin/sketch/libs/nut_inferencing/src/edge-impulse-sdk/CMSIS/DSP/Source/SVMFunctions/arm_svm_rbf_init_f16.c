#line 1 "/home/ubuntu/proj/esp32_voc/esp32_voc/libs/nut_inferencing/src/edge-impulse-sdk/CMSIS/DSP/Source/SVMFunctions/arm_svm_rbf_init_f16.c"
#include "edge-impulse-sdk/dsp/config.hpp"
#if EIDSP_LOAD_CMSIS_DSP_SOURCES
/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_svm_rbf_init_f16.c
 * Description:  SVM Radial Basis Function Instance Initialization
 *
 * $Date:        23 April 2021
 * $Revision:    V1.9.0
 *
 * Target Processor: Cortex-M and Cortex-A cores
 * -------------------------------------------------------------------- */
/*
 * Copyright (C) 2010-2021 ARM Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "edge-impulse-sdk/CMSIS/DSP/Include/dsp/svm_functions_f16.h"

#if defined(ARM_FLOAT16_SUPPORTED)

#include <limits.h>
#include <math.h>

/**
  @ingroup groupSVM
 */



/**
 * @addtogroup rbfsvm
 * @{
 */


/**
 * @brief        SVM radial basis function instance init function
 *
 * Classes are integer used as output of the function (instead of having -1,1
 * as class values).
 *
 * @param[in]    S                      points to an instance of the polynomial SVM structure.
 * @param[in]    nbOfSupportVectors     Number of support vectors
 * @param[in]    vectorDimension        Dimension of vector space
 * @param[in]    intercept              Intercept
 * @param[in]    dualCoefficients       Array of dual coefficients
 * @param[in]    supportVectors         Array of support vectors
 * @param[in]    classes                Array of 2 classes ID
 * @param[in]    gamma                  gamma (scikit-learn terminology)
 * @return none.
 *
 */

void arm_svm_rbf_init_f16(arm_svm_rbf_instance_f16 *S, 
  uint32_t nbOfSupportVectors,
  uint32_t vectorDimension,
  float16_t intercept,
  const float16_t *dualCoefficients,
  const float16_t *supportVectors,
  const int32_t *classes,
  float16_t gamma
  )
{
   S->nbOfSupportVectors = nbOfSupportVectors;
   S->vectorDimension = vectorDimension;
   S->intercept = intercept;
   S->dualCoefficients = dualCoefficients;
   S->supportVectors = supportVectors;
   S->classes = classes;
   S->gamma = gamma;
}



/**
 * @} end of rbfsvm group
 */

#endif /* #if defined(ARM_FLOAT16_SUPPORTED) */ 


#endif // EIDSP_LOAD_CMSIS_DSP_SOURCES