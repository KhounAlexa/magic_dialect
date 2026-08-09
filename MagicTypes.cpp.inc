/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* TypeDef Definitions                                                        *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#ifdef GET_TYPEDEF_LIST
#undef GET_TYPEDEF_LIST

::magic::MagicStateType,
::magic::QubitType

#endif  // GET_TYPEDEF_LIST

#ifdef GET_TYPEDEF_CLASSES
#undef GET_TYPEDEF_CLASSES

static ::mlir::OptionalParseResult generatedTypeParser(::mlir::AsmParser &parser, ::llvm::StringRef *mnemonic, ::mlir::Type &value) {
  return ::mlir::AsmParser::KeywordSwitch<::mlir::OptionalParseResult>(parser)
    .Case(::magic::MagicStateType::getMnemonic(), [&](llvm::StringRef, llvm::SMLoc) {
      value = ::magic::MagicStateType::get(parser.getContext());
      return ::mlir::success(!!value);
    })
    .Case(::magic::QubitType::getMnemonic(), [&](llvm::StringRef, llvm::SMLoc) {
      value = ::magic::QubitType::get(parser.getContext());
      return ::mlir::success(!!value);
    })
    .Default([&](llvm::StringRef keyword, llvm::SMLoc) {
      *mnemonic = keyword;
      return std::nullopt;
    });
}

static ::mlir::LogicalResult generatedTypePrinter(::mlir::Type def, ::mlir::AsmPrinter &printer) {
  return ::llvm::TypeSwitch<::mlir::Type, ::mlir::LogicalResult>(def)    .Case<::magic::MagicStateType>([&](auto t) {
      printer << ::magic::MagicStateType::getMnemonic();
      return ::mlir::success();
    })
    .Case<::magic::QubitType>([&](auto t) {
      printer << ::magic::QubitType::getMnemonic();
      return ::mlir::success();
    })
    .Default([](auto) { return ::mlir::failure(); });
}

namespace magic {
} // namespace magic
MLIR_DEFINE_EXPLICIT_TYPE_ID(::magic::MagicStateType)
namespace magic {
} // namespace magic
MLIR_DEFINE_EXPLICIT_TYPE_ID(::magic::QubitType)
namespace magic {

/// Parse a type registered to this dialect.
::mlir::Type MagicDialect::parseType(::mlir::DialectAsmParser &parser) const {
  ::llvm::SMLoc typeLoc = parser.getCurrentLocation();
  ::llvm::StringRef mnemonic;
  ::mlir::Type genType;
  auto parseResult = generatedTypeParser(parser, &mnemonic, genType);
  if (parseResult.has_value())
    return genType;
  
  parser.emitError(typeLoc) << "unknown  type `"
      << mnemonic << "` in dialect `" << getNamespace() << "`";
  return {};
}
/// Print a type registered to this dialect.
void MagicDialect::printType(::mlir::Type type,
                    ::mlir::DialectAsmPrinter &printer) const {
  if (::mlir::succeeded(generatedTypePrinter(type, printer)))
    return;
  
}
} // namespace magic

#endif  // GET_TYPEDEF_CLASSES

