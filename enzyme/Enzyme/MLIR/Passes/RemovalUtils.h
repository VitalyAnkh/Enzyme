//===- RemovalUtils.h - Utilities to remove Enzyme ops -------* C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#pragma once

#include "Dialect/Ops.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/IRMapping.h"

namespace mlir {
namespace enzyme {

/// Information about a cache, each cache init should have one corresponding
/// push and pop.
struct CacheInfo {
  enzyme::InitOp initOp;
  enzyme::PushOp pushOp;
  enzyme::PopOp popOp;

  CacheInfo() {
    initOp = nullptr;
    pushOp = nullptr;
    popOp = nullptr;
  }
  CacheInfo(Value cache) {
    initOp = cache.getDefiningOp<enzyme::InitOp>();
    unsigned nusers = 0;
    for (auto user : cache.getUsers()) {
      nusers++;
      if (!popOp)
        popOp = dyn_cast<enzyme::PopOp>(user);
      if (!pushOp)
        pushOp = dyn_cast<enzyme::PushOp>(user);
    }
    assert(nusers == 2); // TODO: support more uses
  }

  Value pushedValue() { return pushOp.getValue(); }
  Type cachedType() {
    return cast<enzyme::CacheType>(initOp.getResult().getType()).getType();
  }

  // Pushed values must be the same
  CacheInfo merge(CacheInfo other, PatternRewriter &rewriter);
};

// Tries to limit the amount of values cache from block `forward` to `reverse`
// using a mincut algorithm and heuristics based on the size of values.
void minCutCache(Block *forward, Block *reverse, SmallVector<CacheInfo> &caches,
                 PatternRewriter &rewriter);

} // namespace enzyme
} // namespace mlir
