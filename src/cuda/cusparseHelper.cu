/**
 * Copyright (c) 2017 Darius Rückert 
 * Licensed under the MIT License.
 * See LICENSE file for more information.
 */

#include "saiga/cuda/cusparseHelper.h"

#include "saiga/util/assert.h"

namespace Saiga {
namespace CUDA {

using std::cout;
using std::endl;


cusparseHandle_t cusparseHandle = 0;
cublasHandle_t cublashandle = 0;

void initBLASSPARSE(){
    if(!isBLASSPARSEInitialized()){
        cublasCreate(&cublashandle);
        cusparseCreate(&cusparseHandle);
    }
}

void destroyBLASSPARSE(){
    if(isBLASSPARSEInitialized()){
        cusparseDestroy(cusparseHandle);
        cublasDestroy(cublashandle);
        cusparseHandle = 0;
        cublashandle = 0;
    }
}

bool isBLASSPARSEInitialized(){
    return cusparseHandle != 0;
}

extern void testCuBLAS();
extern void testCuSparse();
void runBLASSPARSETests(){
    testCuBLAS();
    testCuSparse();
}

}
}
