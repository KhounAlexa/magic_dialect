/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* TypeDef Declarations                                                       *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#ifdef GET_TYPEDEF_CLASSES
#undef GET_TYPEDEF_CLASSES


namespace mlir {
class AsmParser;
class AsmPrinter;
} // namespace mlir
namespace magic {
class MagicStateType;
class QubitType;
class MagicStateType : public ::mlir::Type::TypeBase<MagicStateType, ::mlir::Type, ::mlir::TypeStorage> {
public:
  using Base::Base;
  static constexpr ::llvm::StringLiteral name = "magic.state";
  static constexpr ::llvm::StringLiteral getMnemonic() {
    return {"state"};
  }

};
class QubitType : public ::mlir::Type::TypeBase<QubitType, ::mlir::Type, ::mlir::TypeStorage> {
public:
  using Base::Base;
  static constexpr ::llvm::StringLiteral name = "magic.qubit";
  static constexpr ::llvm::StringLiteral getMnemonic() {
    return {"qubit"};
  }

};
} // namespace magic
MLIR_DECLARE_EXPLICIT_TYPE_ID(::magic::MagicStateType)
MLIR_DECLARE_EXPLICIT_TYPE_ID(::magic::QubitType)

#endif  // GET_TYPEDEF_CLASSES

