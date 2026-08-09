/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* Op Definitions                                                             *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|* From: MagicOps.td                                                          *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#ifdef GET_OP_LIST
#undef GET_OP_LIST

::magic::ConsumeOp,
::magic::CreateOp,
::magic::MoveOp,
::magic::TGateOp
#endif  // GET_OP_LIST

#ifdef GET_OP_CLASSES
#undef GET_OP_CLASSES


//===----------------------------------------------------------------------===//
// Local Utility Method Definitions
//===----------------------------------------------------------------------===//

namespace magic {

static ::mlir::LogicalResult __mlir_ods_local_type_constraint_MagicOps0(
    ::mlir::Operation *op, ::mlir::Type type, ::llvm::StringRef valueKind,
    unsigned valueIndex) {
  if (!((::llvm::isa<::magic::MagicStateType>(type)))) {
    return op->emitOpError(valueKind) << " #" << valueIndex
        << " must be A magic state qubit resource (logical level), but got " << type;
  }
  return ::mlir::success();
}

static ::mlir::LogicalResult __mlir_ods_local_type_constraint_MagicOps1(
    ::mlir::Operation *op, ::mlir::Type type, ::llvm::StringRef valueKind,
    unsigned valueIndex) {
  if (!((::llvm::isa<::magic::QubitType>(type)))) {
    return op->emitOpError(valueKind) << " #" << valueIndex
        << " must be A logical qubit, but got " << type;
  }
  return ::mlir::success();
}

static ::mlir::LogicalResult __mlir_ods_local_type_constraint_MagicOps2(
    ::mlir::Operation *op, ::mlir::Type type, ::llvm::StringRef valueKind,
    unsigned valueIndex) {
  if (!((type.isSignlessInteger(1)))) {
    return op->emitOpError(valueKind) << " #" << valueIndex
        << " must be 1-bit signless integer, but got " << type;
  }
  return ::mlir::success();
}
} // namespace magic
namespace magic {

//===----------------------------------------------------------------------===//
// ::magic::ConsumeOp definitions
//===----------------------------------------------------------------------===//

namespace detail {
ConsumeOpGenericAdaptorBase::ConsumeOpGenericAdaptorBase(::mlir::DictionaryAttr attrs, const ::mlir::EmptyProperties &properties, ::mlir::RegionRange regions) : odsAttrs(attrs), odsRegions(regions) {  if (odsAttrs)
    odsOpName.emplace("magic.consume", odsAttrs.getContext());
}

ConsumeOpGenericAdaptorBase::ConsumeOpGenericAdaptorBase(ConsumeOp op) : ConsumeOpGenericAdaptorBase(op->getAttrDictionary(), op.getProperties(), op->getRegions()) {}

std::pair<unsigned, unsigned> ConsumeOpGenericAdaptorBase::getODSOperandIndexAndLength(unsigned index, unsigned odsOperandsSize) {
  return {index, 1};
}

::mlir::DictionaryAttr ConsumeOpGenericAdaptorBase::getAttributes() {
  return odsAttrs;
}

} // namespace detail
ConsumeOpAdaptor::ConsumeOpAdaptor(ConsumeOp op) : ConsumeOpGenericAdaptor(op->getOperands(), op) {}

::mlir::LogicalResult ConsumeOpAdaptor::verify(::mlir::Location loc) {
  return ::mlir::success();
}

void ConsumeOp::getAsmResultNames(::mlir::OpAsmSetValueNameFn setNameFn) {
  auto resultGroup0 = getODSResults(0);
  if (!resultGroup0.empty())
    setNameFn(*resultGroup0.begin(), "updated_qubit");
  auto resultGroup1 = getODSResults(1);
  if (!resultGroup1.empty())
    setNameFn(*resultGroup1.begin(), "classical_bit");
}

std::pair<unsigned, unsigned> ConsumeOp::getODSOperandIndexAndLength(unsigned index) {
  return {index, 1};
}

::mlir::Operation::operand_range ConsumeOp::getODSOperands(unsigned index) {
  auto valueRange = getODSOperandIndexAndLength(index);
  return {std::next(getOperation()->operand_begin(), valueRange.first),
           std::next(getOperation()->operand_begin(), valueRange.first + valueRange.second)};
}

::mlir::TypedValue<::magic::MagicStateType> ConsumeOp::getMagicState() {
  return ::llvm::cast<::mlir::TypedValue<::magic::MagicStateType>>(*getODSOperands(0).begin());
}

::mlir::TypedValue<::magic::QubitType> ConsumeOp::getQubit() {
  return ::llvm::cast<::mlir::TypedValue<::magic::QubitType>>(*getODSOperands(1).begin());
}

::mlir::OpOperand &ConsumeOp::getMagicStateMutable() {
  auto range = getODSOperandIndexAndLength(0);
  return getOperation()->getOpOperand(range.first);
}

::mlir::OpOperand &ConsumeOp::getQubitMutable() {
  auto range = getODSOperandIndexAndLength(1);
  return getOperation()->getOpOperand(range.first);
}

std::pair<unsigned, unsigned> ConsumeOp::getODSResultIndexAndLength(unsigned index) {
  return {index, 1};
}

::mlir::Operation::result_range ConsumeOp::getODSResults(unsigned index) {
  auto valueRange = getODSResultIndexAndLength(index);
  return {std::next(getOperation()->result_begin(), valueRange.first),
           std::next(getOperation()->result_begin(), valueRange.first + valueRange.second)};
}

::mlir::TypedValue<::magic::QubitType> ConsumeOp::getUpdatedQubit() {
  return ::llvm::cast<::mlir::TypedValue<::magic::QubitType>>(*getODSResults(0).begin());
}

::mlir::TypedValue<::mlir::IntegerType> ConsumeOp::getClassicalBit() {
  return ::llvm::cast<::mlir::TypedValue<::mlir::IntegerType>>(*getODSResults(1).begin());
}

void ConsumeOp::build(::mlir::OpBuilder &odsBuilder, ::mlir::OperationState &odsState, ::mlir::Type updated_qubit, ::mlir::Type classical_bit, ::mlir::Value magic_state, ::mlir::Value qubit) {
  odsState.addOperands(magic_state);
  odsState.addOperands(qubit);
  odsState.addTypes(updated_qubit);
  odsState.addTypes(classical_bit);
}

void ConsumeOp::build(::mlir::OpBuilder &odsBuilder, ::mlir::OperationState &odsState, ::mlir::TypeRange resultTypes, ::mlir::Value magic_state, ::mlir::Value qubit) {
  odsState.addOperands(magic_state);
  odsState.addOperands(qubit);
  assert(resultTypes.size() == 2u && "mismatched number of results");
  odsState.addTypes(resultTypes);
}

void ConsumeOp::build(::mlir::OpBuilder &, ::mlir::OperationState &odsState, ::mlir::TypeRange resultTypes, ::mlir::ValueRange operands, ::llvm::ArrayRef<::mlir::NamedAttribute> attributes) {
  assert(operands.size() == 2u && "mismatched number of parameters");
  odsState.addOperands(operands);
  odsState.addAttributes(attributes);
  assert(resultTypes.size() == 2u && "mismatched number of return types");
  odsState.addTypes(resultTypes);
}

::mlir::LogicalResult ConsumeOp::verifyInvariantsImpl() {
  {
    unsigned index = 0; (void)index;
    auto valueGroup0 = getODSOperands(0);

    for (auto v : valueGroup0) {
      if (::mlir::failed(__mlir_ods_local_type_constraint_MagicOps0(*this, v.getType(), "operand", index++)))
        return ::mlir::failure();
    }
    auto valueGroup1 = getODSOperands(1);

    for (auto v : valueGroup1) {
      if (::mlir::failed(__mlir_ods_local_type_constraint_MagicOps1(*this, v.getType(), "operand", index++)))
        return ::mlir::failure();
    }
  }
  {
    unsigned index = 0; (void)index;
    auto valueGroup0 = getODSResults(0);

    for (auto v : valueGroup0) {
      if (::mlir::failed(__mlir_ods_local_type_constraint_MagicOps1(*this, v.getType(), "result", index++)))
        return ::mlir::failure();
    }
    auto valueGroup1 = getODSResults(1);

    for (auto v : valueGroup1) {
      if (::mlir::failed(__mlir_ods_local_type_constraint_MagicOps2(*this, v.getType(), "result", index++)))
        return ::mlir::failure();
    }
  }
  return ::mlir::success();
}

::mlir::LogicalResult ConsumeOp::verifyInvariants() {
  if(::mlir::succeeded(verifyInvariantsImpl()) && ::mlir::succeeded(verify()))
    return ::mlir::success();
  return ::mlir::failure();
}

} // namespace magic
MLIR_DEFINE_EXPLICIT_TYPE_ID(::magic::ConsumeOp)

namespace magic {

//===----------------------------------------------------------------------===//
// ::magic::CreateOp definitions
//===----------------------------------------------------------------------===//

namespace detail {
CreateOpGenericAdaptorBase::CreateOpGenericAdaptorBase(::mlir::DictionaryAttr attrs, const ::mlir::EmptyProperties &properties, ::mlir::RegionRange regions) : odsAttrs(attrs), odsRegions(regions) {  if (odsAttrs)
    odsOpName.emplace("magic.create", odsAttrs.getContext());
}

CreateOpGenericAdaptorBase::CreateOpGenericAdaptorBase(CreateOp op) : CreateOpGenericAdaptorBase(op->getAttrDictionary(), op.getProperties(), op->getRegions()) {}

std::pair<unsigned, unsigned> CreateOpGenericAdaptorBase::getODSOperandIndexAndLength(unsigned index, unsigned odsOperandsSize) {
  return {index, 1};
}

::mlir::DictionaryAttr CreateOpGenericAdaptorBase::getAttributes() {
  return odsAttrs;
}

} // namespace detail
CreateOpAdaptor::CreateOpAdaptor(CreateOp op) : CreateOpGenericAdaptor(op->getOperands(), op) {}

::mlir::LogicalResult CreateOpAdaptor::verify(::mlir::Location loc) {
  return ::mlir::success();
}

std::pair<unsigned, unsigned> CreateOp::getODSOperandIndexAndLength(unsigned index) {
  return {index, 1};
}

::mlir::Operation::operand_range CreateOp::getODSOperands(unsigned index) {
  auto valueRange = getODSOperandIndexAndLength(index);
  return {std::next(getOperation()->operand_begin(), valueRange.first),
           std::next(getOperation()->operand_begin(), valueRange.first + valueRange.second)};
}

std::pair<unsigned, unsigned> CreateOp::getODSResultIndexAndLength(unsigned index) {
  return {index, 1};
}

::mlir::Operation::result_range CreateOp::getODSResults(unsigned index) {
  auto valueRange = getODSResultIndexAndLength(index);
  return {std::next(getOperation()->result_begin(), valueRange.first),
           std::next(getOperation()->result_begin(), valueRange.first + valueRange.second)};
}

::mlir::TypedValue<::magic::MagicStateType> CreateOp::getResult() {
  return ::llvm::cast<::mlir::TypedValue<::magic::MagicStateType>>(*getODSResults(0).begin());
}

void CreateOp::build(::mlir::OpBuilder &odsBuilder, ::mlir::OperationState &odsState, ::mlir::Type result) {
  odsState.addTypes(result);
}

void CreateOp::build(::mlir::OpBuilder &odsBuilder, ::mlir::OperationState &odsState, ::mlir::TypeRange resultTypes) {
  assert(resultTypes.size() == 1u && "mismatched number of results");
  odsState.addTypes(resultTypes);
}

void CreateOp::build(::mlir::OpBuilder &, ::mlir::OperationState &odsState, ::mlir::TypeRange resultTypes, ::mlir::ValueRange operands, ::llvm::ArrayRef<::mlir::NamedAttribute> attributes) {
  assert(operands.size() == 0u && "mismatched number of parameters");
  odsState.addOperands(operands);
  odsState.addAttributes(attributes);
  assert(resultTypes.size() == 1u && "mismatched number of return types");
  odsState.addTypes(resultTypes);
}

::mlir::LogicalResult CreateOp::verifyInvariantsImpl() {
  {
    unsigned index = 0; (void)index;
    auto valueGroup0 = getODSResults(0);

    for (auto v : valueGroup0) {
      if (::mlir::failed(__mlir_ods_local_type_constraint_MagicOps0(*this, v.getType(), "result", index++)))
        return ::mlir::failure();
    }
  }
  return ::mlir::success();
}

::mlir::LogicalResult CreateOp::verifyInvariants() {
  if(::mlir::succeeded(verifyInvariantsImpl()) && ::mlir::succeeded(verify()))
    return ::mlir::success();
  return ::mlir::failure();
}

} // namespace magic
MLIR_DEFINE_EXPLICIT_TYPE_ID(::magic::CreateOp)

namespace magic {

//===----------------------------------------------------------------------===//
// ::magic::MoveOp definitions
//===----------------------------------------------------------------------===//

namespace detail {
MoveOpGenericAdaptorBase::MoveOpGenericAdaptorBase(::mlir::DictionaryAttr attrs, const ::mlir::EmptyProperties &properties, ::mlir::RegionRange regions) : odsAttrs(attrs), odsRegions(regions) {  if (odsAttrs)
    odsOpName.emplace("magic.move", odsAttrs.getContext());
}

MoveOpGenericAdaptorBase::MoveOpGenericAdaptorBase(MoveOp op) : MoveOpGenericAdaptorBase(op->getAttrDictionary(), op.getProperties(), op->getRegions()) {}

std::pair<unsigned, unsigned> MoveOpGenericAdaptorBase::getODSOperandIndexAndLength(unsigned index, unsigned odsOperandsSize) {
  return {index, 1};
}

::mlir::DictionaryAttr MoveOpGenericAdaptorBase::getAttributes() {
  return odsAttrs;
}

} // namespace detail
MoveOpAdaptor::MoveOpAdaptor(MoveOp op) : MoveOpGenericAdaptor(op->getOperands(), op) {}

::mlir::LogicalResult MoveOpAdaptor::verify(::mlir::Location loc) {
  return ::mlir::success();
}

std::pair<unsigned, unsigned> MoveOp::getODSOperandIndexAndLength(unsigned index) {
  return {index, 1};
}

::mlir::Operation::operand_range MoveOp::getODSOperands(unsigned index) {
  auto valueRange = getODSOperandIndexAndLength(index);
  return {std::next(getOperation()->operand_begin(), valueRange.first),
           std::next(getOperation()->operand_begin(), valueRange.first + valueRange.second)};
}

::mlir::TypedValue<::magic::MagicStateType> MoveOp::getValue() {
  return ::llvm::cast<::mlir::TypedValue<::magic::MagicStateType>>(*getODSOperands(0).begin());
}

::mlir::OpOperand &MoveOp::getValueMutable() {
  auto range = getODSOperandIndexAndLength(0);
  return getOperation()->getOpOperand(range.first);
}

std::pair<unsigned, unsigned> MoveOp::getODSResultIndexAndLength(unsigned index) {
  return {index, 1};
}

::mlir::Operation::result_range MoveOp::getODSResults(unsigned index) {
  auto valueRange = getODSResultIndexAndLength(index);
  return {std::next(getOperation()->result_begin(), valueRange.first),
           std::next(getOperation()->result_begin(), valueRange.first + valueRange.second)};
}

::mlir::TypedValue<::magic::MagicStateType> MoveOp::getResult() {
  return ::llvm::cast<::mlir::TypedValue<::magic::MagicStateType>>(*getODSResults(0).begin());
}

void MoveOp::build(::mlir::OpBuilder &odsBuilder, ::mlir::OperationState &odsState, ::mlir::Type result, ::mlir::Value value) {
  odsState.addOperands(value);
  odsState.addTypes(result);
}

void MoveOp::build(::mlir::OpBuilder &odsBuilder, ::mlir::OperationState &odsState, ::mlir::TypeRange resultTypes, ::mlir::Value value) {
  odsState.addOperands(value);
  assert(resultTypes.size() == 1u && "mismatched number of results");
  odsState.addTypes(resultTypes);
}

void MoveOp::build(::mlir::OpBuilder &, ::mlir::OperationState &odsState, ::mlir::TypeRange resultTypes, ::mlir::ValueRange operands, ::llvm::ArrayRef<::mlir::NamedAttribute> attributes) {
  assert(operands.size() == 1u && "mismatched number of parameters");
  odsState.addOperands(operands);
  odsState.addAttributes(attributes);
  assert(resultTypes.size() == 1u && "mismatched number of return types");
  odsState.addTypes(resultTypes);
}

::mlir::LogicalResult MoveOp::verifyInvariantsImpl() {
  {
    unsigned index = 0; (void)index;
    auto valueGroup0 = getODSOperands(0);

    for (auto v : valueGroup0) {
      if (::mlir::failed(__mlir_ods_local_type_constraint_MagicOps0(*this, v.getType(), "operand", index++)))
        return ::mlir::failure();
    }
  }
  {
    unsigned index = 0; (void)index;
    auto valueGroup0 = getODSResults(0);

    for (auto v : valueGroup0) {
      if (::mlir::failed(__mlir_ods_local_type_constraint_MagicOps0(*this, v.getType(), "result", index++)))
        return ::mlir::failure();
    }
  }
  return ::mlir::success();
}

::mlir::LogicalResult MoveOp::verifyInvariants() {
  if(::mlir::succeeded(verifyInvariantsImpl()) && ::mlir::succeeded(verify()))
    return ::mlir::success();
  return ::mlir::failure();
}

} // namespace magic
MLIR_DEFINE_EXPLICIT_TYPE_ID(::magic::MoveOp)

namespace magic {

//===----------------------------------------------------------------------===//
// ::magic::TGateOp definitions
//===----------------------------------------------------------------------===//

namespace detail {
TGateOpGenericAdaptorBase::TGateOpGenericAdaptorBase(::mlir::DictionaryAttr attrs, const ::mlir::EmptyProperties &properties, ::mlir::RegionRange regions) : odsAttrs(attrs), odsRegions(regions) {  if (odsAttrs)
    odsOpName.emplace("magic.tgate", odsAttrs.getContext());
}

TGateOpGenericAdaptorBase::TGateOpGenericAdaptorBase(TGateOp op) : TGateOpGenericAdaptorBase(op->getAttrDictionary(), op.getProperties(), op->getRegions()) {}

std::pair<unsigned, unsigned> TGateOpGenericAdaptorBase::getODSOperandIndexAndLength(unsigned index, unsigned odsOperandsSize) {
  return {index, 1};
}

::mlir::DictionaryAttr TGateOpGenericAdaptorBase::getAttributes() {
  return odsAttrs;
}

} // namespace detail
TGateOpAdaptor::TGateOpAdaptor(TGateOp op) : TGateOpGenericAdaptor(op->getOperands(), op) {}

::mlir::LogicalResult TGateOpAdaptor::verify(::mlir::Location loc) {
  return ::mlir::success();
}

std::pair<unsigned, unsigned> TGateOp::getODSOperandIndexAndLength(unsigned index) {
  return {index, 1};
}

::mlir::Operation::operand_range TGateOp::getODSOperands(unsigned index) {
  auto valueRange = getODSOperandIndexAndLength(index);
  return {std::next(getOperation()->operand_begin(), valueRange.first),
           std::next(getOperation()->operand_begin(), valueRange.first + valueRange.second)};
}

::mlir::TypedValue<::magic::QubitType> TGateOp::getQubit() {
  return ::llvm::cast<::mlir::TypedValue<::magic::QubitType>>(*getODSOperands(0).begin());
}

::mlir::OpOperand &TGateOp::getQubitMutable() {
  auto range = getODSOperandIndexAndLength(0);
  return getOperation()->getOpOperand(range.first);
}

std::pair<unsigned, unsigned> TGateOp::getODSResultIndexAndLength(unsigned index) {
  return {index, 1};
}

::mlir::Operation::result_range TGateOp::getODSResults(unsigned index) {
  auto valueRange = getODSResultIndexAndLength(index);
  return {std::next(getOperation()->result_begin(), valueRange.first),
           std::next(getOperation()->result_begin(), valueRange.first + valueRange.second)};
}

::mlir::TypedValue<::magic::QubitType> TGateOp::getResult() {
  return ::llvm::cast<::mlir::TypedValue<::magic::QubitType>>(*getODSResults(0).begin());
}

void TGateOp::build(::mlir::OpBuilder &odsBuilder, ::mlir::OperationState &odsState, ::mlir::Type result, ::mlir::Value qubit) {
  odsState.addOperands(qubit);
  odsState.addTypes(result);
}

void TGateOp::build(::mlir::OpBuilder &odsBuilder, ::mlir::OperationState &odsState, ::mlir::TypeRange resultTypes, ::mlir::Value qubit) {
  odsState.addOperands(qubit);
  assert(resultTypes.size() == 1u && "mismatched number of results");
  odsState.addTypes(resultTypes);
}

void TGateOp::build(::mlir::OpBuilder &, ::mlir::OperationState &odsState, ::mlir::TypeRange resultTypes, ::mlir::ValueRange operands, ::llvm::ArrayRef<::mlir::NamedAttribute> attributes) {
  assert(operands.size() == 1u && "mismatched number of parameters");
  odsState.addOperands(operands);
  odsState.addAttributes(attributes);
  assert(resultTypes.size() == 1u && "mismatched number of return types");
  odsState.addTypes(resultTypes);
}

::mlir::LogicalResult TGateOp::verifyInvariantsImpl() {
  {
    unsigned index = 0; (void)index;
    auto valueGroup0 = getODSOperands(0);

    for (auto v : valueGroup0) {
      if (::mlir::failed(__mlir_ods_local_type_constraint_MagicOps1(*this, v.getType(), "operand", index++)))
        return ::mlir::failure();
    }
  }
  {
    unsigned index = 0; (void)index;
    auto valueGroup0 = getODSResults(0);

    for (auto v : valueGroup0) {
      if (::mlir::failed(__mlir_ods_local_type_constraint_MagicOps1(*this, v.getType(), "result", index++)))
        return ::mlir::failure();
    }
  }
  return ::mlir::success();
}

::mlir::LogicalResult TGateOp::verifyInvariants() {
  return verifyInvariantsImpl();
}

} // namespace magic
MLIR_DEFINE_EXPLICIT_TYPE_ID(::magic::TGateOp)


#endif  // GET_OP_CLASSES

