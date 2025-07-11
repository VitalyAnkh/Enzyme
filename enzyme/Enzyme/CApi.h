//===- CApi.h - Enzyme API exported to C for external use      -----------===//
//
//                             Enzyme Project
//
// Part of the Enzyme Project, under the Apache License v2.0 with LLVM
// Exceptions. See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// If using this code in an academic setting, please cite the following:
// @incollection{enzymeNeurips,
// title = {Instead of Rewriting Foreign Code for Machine Learning,
//          Automatically Synthesize Fast Gradients},
// author = {Moses, William S. and Churavy, Valentin},
// booktitle = {Advances in Neural Information Processing Systems 33},
// year = {2020},
// note = {To appear in},
// }
//
//===----------------------------------------------------------------------===//
//
// This file declares various utility functions of Enzyme for access via C
//
//===----------------------------------------------------------------------===//
#ifndef ENZYME_CAPI_H
#define ENZYME_CAPI_H

#include "llvm-c/Core.h"
#include "llvm-c/DataTypes.h"
// #include "llvm-c/Initialization.h"
#include "llvm-c/Target.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct EnzymeOpaqueTypeAnalysis;
typedef struct EnzymeOpaqueTypeAnalysis *EnzymeTypeAnalysisRef;

struct EnzymeOpaqueLogic;
typedef struct EnzymeOpaqueLogic *EnzymeLogicRef;

struct EnzymeOpaqueAugmentedReturn;
typedef struct EnzymeOpaqueAugmentedReturn *EnzymeAugmentedReturnPtr;

struct EnzymeOpaqueTraceInterface;
typedef struct EnzymeOpaqueTraceInterface *EnzymeTraceInterfaceRef;

struct IntList {
  int64_t *data;
  size_t size;
};

typedef enum {
  DT_Anything = 0,
  DT_Integer = 1,
  DT_Pointer = 2,
  DT_Half = 3,
  DT_Float = 4,
  DT_Double = 5,
  DT_Unknown = 6,
  DT_X86_FP80 = 7,
  DT_BFloat16 = 8,
} CConcreteType;

struct CDataPair {
  struct IntList offsets;
  CConcreteType datatype;
};

/*
struct CTypeTree {
  struct CDataPair *data;
  size_t size;
};
*/

typedef enum {
  VT_None = 0,
  VT_Primal = 1,
  VT_Shadow = 2,
  VT_Both = VT_Primal | VT_Shadow,
} CValueType;

struct EnzymeTypeTree;
typedef struct EnzymeTypeTree *CTypeTreeRef;
CTypeTreeRef EnzymeNewTypeTree();
CTypeTreeRef EnzymeNewTypeTreeCT(CConcreteType, LLVMContextRef ctx);
CTypeTreeRef EnzymeNewTypeTreeTR(CTypeTreeRef);
void EnzymeFreeTypeTree(CTypeTreeRef CTT);
uint8_t EnzymeSetTypeTree(CTypeTreeRef dst, CTypeTreeRef src);
uint8_t EnzymeMergeTypeTree(CTypeTreeRef dst, CTypeTreeRef src);
void EnzymeTypeTreeOnlyEq(CTypeTreeRef dst, int64_t x);
void EnzymeTypeTreeData0Eq(CTypeTreeRef dst);
void EnzymeTypeTreeShiftIndiciesEq(CTypeTreeRef dst, const char *datalayout,
                                   int64_t offset, int64_t maxSize,
                                   uint64_t addOffset);
const char *EnzymeTypeTreeToString(CTypeTreeRef src);
void EnzymeTypeTreeToStringFree(const char *cstr);

void EnzymeSetCLBool(void *, uint8_t);
void EnzymeSetCLInteger(void *, int64_t);
void EnzymeSetCLString(void *, const char *);

struct CFnTypeInfo {
  /// Types of arguments, assumed of size len(Arguments)
  CTypeTreeRef *Arguments;

  /// Type of return
  CTypeTreeRef Return;

  /// The specific constant(s) known to represented by an argument, if constant
  // map is [arg number] => list
  struct IntList *KnownValues;
};

typedef enum {
  DFT_OUT_DIFF = 0,  // add differential to an output struct. Only for scalar
                     // values in ReverseMode variants.
  DFT_DUP_ARG = 1,   // duplicate the argument and store differential inside.
                     // For references, pointers, or integers in ReverseMode
                     // variants. For all types in ForwardMode variants.
  DFT_CONSTANT = 2,  // no differential. Usable everywhere.
  DFT_DUP_NONEED = 3 // duplicate this argument and store differential inside,
                     // but don't need the forward. Same as DUP_ARG otherwise.
} CDIFFE_TYPE;

typedef enum { BT_SCALAR = 0, BT_VECTOR = 1 } CBATCH_TYPE;

typedef enum {
  DEM_ForwardMode = 0,
  DEM_ReverseModePrimal = 1,
  DEM_ReverseModeGradient = 2,
  DEM_ReverseModeCombined = 3,
  DEM_ForwardModeSplit = 4,
  DEM_ForwardModeError = 5
} CDerivativeMode;

typedef enum {
  DEM_Trace = 0,
  DEM_Condition = 1,
} CProbProgMode;

typedef uint8_t (*CustomRuleType)(int /*direction*/, CTypeTreeRef /*return*/,
                                  CTypeTreeRef * /*args*/,
                                  struct IntList * /*knownValues*/,
                                  size_t /*numArgs*/, LLVMValueRef,
                                  void * /*TA*/);
EnzymeTypeAnalysisRef CreateTypeAnalysis(EnzymeLogicRef Log,
                                         char **customRuleNames,
                                         CustomRuleType *customRules,
                                         size_t numRules);
void ClearTypeAnalysis(EnzymeTypeAnalysisRef);
void FreeTypeAnalysis(EnzymeTypeAnalysisRef);

EnzymeTraceInterfaceRef FindEnzymeStaticTraceInterface(LLVMModuleRef M);
EnzymeTraceInterfaceRef CreateEnzymeStaticTraceInterface(
    LLVMContextRef C, LLVMValueRef getTraceFunction,
    LLVMValueRef getChoiceFunction, LLVMValueRef insertCallFunction,
    LLVMValueRef insertChoiceFunction, LLVMValueRef insertArgumentFunction,
    LLVMValueRef insertReturnFunction, LLVMValueRef insertFunctionFunction,
    LLVMValueRef insertChoiceGradientFunction,
    LLVMValueRef insertArgumentGradientFunction, LLVMValueRef newTraceFunction,
    LLVMValueRef freeTraceFunction, LLVMValueRef hasCallFunction,
    LLVMValueRef hasChoiceFunction);
EnzymeTraceInterfaceRef
CreateEnzymeDynamicTraceInterface(LLVMValueRef interface, LLVMValueRef F);
EnzymeLogicRef CreateEnzymeLogic(uint8_t PostOpt);
void ClearEnzymeLogic(EnzymeLogicRef);
void FreeEnzymeLogic(EnzymeLogicRef);

void EnzymeExtractReturnInfo(EnzymeAugmentedReturnPtr ret, int64_t *data,
                             uint8_t *existed, size_t len);

LLVMValueRef
EnzymeExtractFunctionFromAugmentation(EnzymeAugmentedReturnPtr ret);
LLVMTypeRef EnzymeExtractTapeTypeFromAugmentation(EnzymeAugmentedReturnPtr ret);

class GradientUtils;
class DiffeGradientUtils;

typedef LLVMValueRef (*CustomShadowAlloc)(LLVMBuilderRef, LLVMValueRef,
                                          size_t /*numArgs*/, LLVMValueRef *,
                                          GradientUtils *);
typedef LLVMValueRef (*CustomShadowFree)(LLVMBuilderRef, LLVMValueRef);

void EnzymeRegisterAllocationHandler(char *Name, CustomShadowAlloc AHandle,
                                     CustomShadowFree FHandle);

typedef uint8_t (*CustomFunctionForward)(LLVMBuilderRef, LLVMValueRef,
                                         GradientUtils *, LLVMValueRef *,
                                         LLVMValueRef *);

typedef uint8_t (*CustomFunctionDiffUse)(LLVMValueRef, const GradientUtils *,
                                         LLVMValueRef, uint8_t, CDerivativeMode,
                                         uint8_t *);

typedef uint8_t (*CustomAugmentedFunctionForward)(LLVMBuilderRef, LLVMValueRef,
                                                  GradientUtils *,
                                                  LLVMValueRef *,
                                                  LLVMValueRef *,
                                                  LLVMValueRef *);

typedef void (*CustomFunctionReverse)(LLVMBuilderRef, LLVMValueRef,
                                      DiffeGradientUtils *, LLVMValueRef);

LLVMValueRef EnzymeCreateForwardDiff(
    EnzymeLogicRef Logic, LLVMValueRef request_req, LLVMBuilderRef request_ip,
    LLVMValueRef todiff, CDIFFE_TYPE retType, CDIFFE_TYPE *constant_args,
    size_t constant_args_size, EnzymeTypeAnalysisRef TA, uint8_t returnValue,
    CDerivativeMode mode, uint8_t freeMemory, uint8_t runtimeActivity,
    uint8_t strongZero, unsigned width, LLVMTypeRef additionalArg,
    CFnTypeInfo typeInfo, uint8_t subsequent_calls_may_write,
    uint8_t *_overwritten_args, size_t overwritten_args_size,
    EnzymeAugmentedReturnPtr augmented);

LLVMValueRef EnzymeCreatePrimalAndGradient(
    EnzymeLogicRef Logic, LLVMValueRef request_req, LLVMBuilderRef request_ip,
    LLVMValueRef todiff, CDIFFE_TYPE retType, CDIFFE_TYPE *constant_args,
    size_t constant_args_size, EnzymeTypeAnalysisRef TA, uint8_t returnValue,
    uint8_t dretUsed, CDerivativeMode mode, uint8_t runtimeActivity,
    uint8_t strongZero, unsigned width, uint8_t freeMemory,
    LLVMTypeRef additionalArg, uint8_t forceAnonymousTape, CFnTypeInfo typeInfo,
    uint8_t subsequent_calls_may_write, uint8_t *_overwritten_args,
    size_t overwritten_args_size, EnzymeAugmentedReturnPtr augmented,
    uint8_t AtomicAdd);

void EnzymeRegisterCallHandler(const char *Name,
                               CustomAugmentedFunctionForward FwdHandle,
                               CustomFunctionReverse RevHandle);

LLVMValueRef EnzymeGradientUtilsNewFromOriginal(GradientUtils *gutils,
                                                LLVMValueRef val);

#ifdef __cplusplus
}
#endif

#endif
