//===- Enzyme.td - Enzyme dialect --------------------------*- tablegen -*-===//
//
// This file is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef ENZYME_DIALECT
#define ENZYME_DIALECT

include "mlir/IR/OpBase.td"
include "mlir/IR/AttrTypeBase.td"

//===----------------------------------------------------------------------===//
// Enzyme dialect definition.
//===----------------------------------------------------------------------===//

def Enzyme_Dialect : Dialect {
  let name = "enzyme";
  let description = [{}];
  let cppNamespace = "::mlir::enzyme";
  let useDefaultAttributePrinterParser = 1;
  let useDefaultTypePrinterParser = 1;

  let extraClassDeclaration = [{
    /// Names of analysis summary attributes
    static StringRef getPointerSummaryAttrName() { return "enzyme.p2p"; }
    static StringRef getAliasSummaryAttrName() { return "enzyme.alias"; }
    static StringRef getDenseActivityAnnotationAttrName() { return "enzyme.denseactive"; }
    static StringRef getSparseActivityAnnotationAttrName() { return "enzyme.sparseactive"; }
  }];
}

//===----------------------------------------------------------------------===//
// Base Enzyme operation definition.
//===----------------------------------------------------------------------===//

class Enzyme_Op<string mnemonic, list<Trait> traits = []>
    : Op<Enzyme_Dialect, mnemonic, traits>;

class Enzyme_Type<string name> : TypeDef<Enzyme_Dialect, name>;

#endif // ENZYME_DIALECT
