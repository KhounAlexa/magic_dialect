#include "Dialect.h"

#include "mlir/IR/DialectImplementation.h"
#include "llvm/ADT/TypeSwitch.h"

using namespace magic;

MagicDialect::MagicDialect(mlir::MLIRContext *context)
    : mlir::Dialect(getDialectNamespace(), context, mlir::TypeID::get<MagicDialect>()) {
  initialize();
}

MagicDialect::~MagicDialect() = default;

void MagicDialect::initialize() {
  addOperations<
#define GET_OP_LIST
#include "MagicOps.cpp.inc"
  >();
  addTypes<
#define GET_TYPEDEF_LIST
#include "MagicTypes.cpp.inc"
  >();
}

#define GET_TYPEDEF_CLASSES
#include "MagicTypes.cpp.inc"

#define GET_OP_CLASSES
#include "MagicOps.cpp.inc"

// Verification: a magic state resource is linear -- once created it must be
// moved exactly once (never abandoned, never forked to more than one move).
::mlir::LogicalResult CreateOp::verify() {
  if (getResult().use_empty()) {
    return emitOpError(
        "created magic state is never moved -- a magic state resource "
        "must be moved and consumed, not abandoned");
  }
  if (!getResult().hasOneUse()) {
    return emitOpError(
        "created magic state is used more than once -- a magic state "
        "resource is linear and can only be moved once");
  }
  if (!mlir::isa<magic::MoveOp>(*getResult().user_begin())) {
    return emitOpError(
        "created magic state escapes to a non-'magic.move' use -- a "
        "magic state resource must be moved before anything else can "
        "happen to it");
  }
  return mlir::success();
}

// Verification: a magic.move must consume a value produced by magic.create,
// and its own result must in turn be consumed exactly once (never abandoned,
// never consumed twice).
::mlir::LogicalResult MoveOp::verify() {
  if (!getValue().getDefiningOp<magic::CreateOp>()) {
    return emitOpError(
        "operand must be produced by a 'magic.create' op -- only a "
        "genuine magic state resource can be moved");
  }
  if (getResult().use_empty()) {
    return emitOpError(
        "moved magic state is never consumed -- a magic state resource "
        "must be consumed exactly once after being moved");
  }
  if (!getResult().hasOneUse()) {
    return emitOpError(
        "moved magic state is consumed more than once -- a magic state "
        "resource is linear and can only be consumed once");
  }
  if (!mlir::isa<magic::ConsumeOp>(*getResult().user_begin())) {
    return emitOpError(
        "moved magic state escapes to a non-'magic.consume' use -- a "
        "magic state resource must be consumed and cannot escape the "
        "create/move/consume chain");
  }
  return mlir::success();
}

// Verification: a magic.consume must consume a value produced by magic.move.
::mlir::LogicalResult ConsumeOp::verify() {
  if (!getMagicState().getDefiningOp<magic::MoveOp>()) {
    return emitOpError(
        "magic state operand must be produced by a 'magic.move' op -- "
        "a magic state must be moved before it can be consumed");
  }
  return mlir::success();
}

MLIR_DEFINE_EXPLICIT_TYPE_ID(::magic::MagicDialect)
