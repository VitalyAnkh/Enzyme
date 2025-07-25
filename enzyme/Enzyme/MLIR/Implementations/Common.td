#ifndef ENZYME_MLIR_IMPLEMENTATIONS_COMMON
#define ENZYME_MLIR_IMPLEMENTATIONS_COMMON

class InactiveOp<string dialect_, string opName_> {
  string dialect = dialect_;
  string opName = opName_;
}

class AllocationOp<string dialect_, string opName_> {
  string dialect = dialect_;
  string opName = opName_;
}

class ControlFlowOp<string dialect_, string opName_, string impl_> {
  string dialect = dialect_;
  string opName = opName_;
  string impl = impl_;
}

class InactiveArgSpec {
  bit asserting;
}
def InactiveArg : InactiveArgSpec {
  bit asserting = 0;
}
def AssertingInactiveArg : InactiveArgSpec {
  bit asserting = 1;
}

class Variadic<string getter_> {
  string getter = getter_;
}

def Unimplemented {

}

class MemoryIdentityOp<string dialect_, string opName_, list<int> ptrargs_, list<int> storedargs_ = [], dag patternToMatch=(Unimplemented), list<dag> reverse_ = []> {
  string dialect = dialect_;
  string opName = opName_;
  dag PatternToMatch = patternToMatch;
  list<int> ptrargs = ptrargs_;
  list<int> storedargs = storedargs_;
  list<dag> reverse = reverse_;
}

class ReadOnlyIdentityOp<string dialect_, string opName_, list<int> ptrargs_, dag patternToMatch=(Unimplemented), list<dag> reverse_ = []> : MemoryIdentityOp<dialect_, opName_, ptrargs_, [], patternToMatch, reverse_>;

class ReturnOp<string dialect_, string opName_> {
  string dialect = dialect_;
  string opName = opName_;
}

class BranchOp<string dialect_, string opName_> {
  string dialect = dialect_;
  string opName = opName_;
}

class RegionTerminatorOp<string dialect_, string opName_> {
  string dialect = dialect_;
  string opName = opName_;
}

class ForwardFromSummedReverseInternal<int unused_> {
  int unused = unused_;
}
def ForwardFromSummedReverse : ForwardFromSummedReverseInternal<0>;


class MLIRDerivative<string dialect_, string opName_, dag patternToMatch, list<dag> resultOps, dag forwardOps=(ForwardFromSummedReverse)> {
  string dialect = dialect_;
  string opName = opName_;
  dag PatternToMatch = patternToMatch;
  list<dag> ArgDerivatives = resultOps;
  dag ArgDuals = forwardOps;
}

class Operation<bit usesPrimal_, bit usesShadow_, bit usesCustom_=0> {
  bit usesPrimal = usesPrimal_;
  bit usesShadow = usesShadow_;
  bit usesCustom = usesCustom_;
}

class DiffeRetIndex<list<int> indices_> {
  list<int> indices = indices_;
}
def DiffeRet : DiffeRetIndex<[-1]>;

def Shadow : Operation</*primal*/0, /*shadow*/1> {
}

class GlobalExpr<bit uses_primal, bit uses_shadow, string val> : Operation<uses_primal, uses_shadow> {
  string value = val;
}

// Class for a dag operator that generates either a or b
// It can then be used with a two or three arguments.
// The two arguments version is (StaticSelect a, b)
// The three arguments version accepts a name as a first argument
// which is then available in the condition as a `Value` under the
// variable `imVal`.
class StaticSelect<string condition_> : Operation</*usesPrimal*/0, /*usesShadow*/0, /*usesCustom*/0> {
  string condition = condition_;
}

class Inst<string mnemonic, string dialect_, string postop_="", string preop_="">  : Operation</*primal*/1, /*shadow*/0> {
  string name = mnemonic;
  string dialect = dialect_;
  string postop = postop_;
  string preop = preop_;
}

def Op {
}

class SubRoutine<dag patternToMatch, dag insts_> : Operation</*primal*/1,/*shadow*/0> {
  dag PatternToMatch = patternToMatch;
  dag insts = insts_;
}

def SelectIfStrongZero : StaticSelect<"gutils->strongZero">;
def SelectIfActive : StaticSelect<"!gutils->isConstantValue(imVal)">;

def SelectIfComplex : StaticSelect<[{
  auto ty = imVal.getType();
  isa<ComplexType>(ty) ||
    isa<TensorType>(ty) &&
      isa<ComplexType>(cast<TensorType>(ty).getElementType());
}]>;

class ConstantFP<string val, string dialect_, string op_, string type_=""> : Operation</*primal*/0, /*shadow*/0> {
  string value = val;
  string dialect = dialect_;
  string opName = op_;
  string type = type_;
}

def ResultTypes : GlobalExpr</*needsprimal*/0, /*needsshadow*/0, "op->getResultTypes()">;

def TypeOf : Operation</*primal*/0, /*shadow*/0> {
}

class ComplexInst<string m> : Inst<m, "complex">;
class ArithInst<string m> : Inst<m, "arith">;
class LlvmInst<string m> : Inst<m, "LLVM">;
class MathInst<string m> : Inst<m, "math">;

def AddF : ArithInst<"AddFOp">;
def SubF : ArithInst<"SubFOp">;
def NegF : ArithInst<"NegFOp">;
def MulF : ArithInst<"MulFOp">;
def DivF : ArithInst<"DivFOp">;
def RemF : ArithInst<"RemFOp">;
def CmpF : ArithInst<"CmpFOp">;
def Arith_Select : ArithInst<"SelectOp">;

def Arith_OEQ : GlobalExpr</*needsprimal*/0, /*needsshadow*/0, "arith::CmpFPredicate::OEQ">;
def Arith_OGE : GlobalExpr</*needsprimal*/0, /*needsshadow*/0, "arith::CmpFPredicate::OGE">;

def CheckedMulF : SubRoutine<(Op $diffret, $x),
                  (
                    SelectIfStrongZero
                    (Arith_Select (CmpF (Arith_OEQ), $diffret, (ConstantFP<"0","arith", "ConstantOp"> $diffret):$zero), $zero, (MulF $diffret, $x)),
                    (MulF $diffret, $x)
                  )>;

def CheckedDivF : SubRoutine<(Op $diffret, $x),
                  (
                    SelectIfStrongZero
                    (Arith_Select (CmpF (Arith_OEQ), $diffret, (ConstantFP<"0","arith", "ConstantOp"> $diffret):$zero), $zero, (DivF $diffret, $x)),
                    (DivF $diffret, $x)
                  )>;

def LlvmCheckedMulF : LlvmInst<"FMulOp">;
def LlvmExpF : LlvmInst<"ExpOp">;

def CosF : MathInst<"CosOp">;
def SinF : MathInst<"SinOp">;
def ExpF : MathInst<"ExpOp">;
def SqrtF : MathInst<"SqrtOp">;

#endif // ENZYME_MLIR_IMPLEMENTATIONS_COMMON
