#include "magic/Dialect.h"

#include "mlir/IR/DialectImplementation.h"
#include "llvm/ADT/TypeSwitch.h"

using namespace magic;

// Constructor, destructor and TypeID definition are generated from
// MagicOps.td (-gen-dialect-defs); only initialize() is hand-written.
#include "magic/MagicDialect.cpp.inc"

void MagicDialect::initialize() {
  addOperations<
#define GET_OP_LIST
#include "magic/MagicOps.cpp.inc"
  >();
  addTypes<
#define GET_TYPEDEF_LIST
#include "magic/MagicTypes.cpp.inc"
  >();
}

#define GET_TYPEDEF_CLASSES
#include "magic/MagicTypes.cpp.inc"

#define GET_OP_CLASSES
#include "magic/MagicOps.cpp.inc"

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

// Verification: a magic.consume must consume a value produced by magic.move,
// and its qubit operand -- like every qubit-consuming op's qubit operand --
// must not be forked (see the shared comment on TGateOp::verify below).
::mlir::LogicalResult ConsumeOp::verify() {
  if (!getMagicState().getDefiningOp<magic::MoveOp>()) {
    return emitOpError(
        "magic state operand must be produced by a 'magic.move' op -- "
        "a magic state must be moved before it can be consumed");
  }
  if (!getQubit().hasOneUse()) {
    return emitOpError(
        "qubit operand is used more than once -- a logical qubit value "
        "cannot be forked; the qubit produced by one gate must be threaded "
        "into exactly one next use, not reused independently by two gates");
  }
  return mlir::success();
}

// Verification: a logical qubit value is linear too, just like a magic
// state -- a single physical qubit cannot independently undergo two
// unrelated gate operations. The value must have exactly one use (this op
// itself, since verify() only runs once this op already IS a use), so it
// is enough to reject any *additional* use.
::mlir::LogicalResult TGateOp::verify() {
  if (!getQubit().hasOneUse()) {
    return emitOpError(
        "qubit operand is used more than once -- a logical qubit value "
        "cannot be forked; the qubit produced by one gate must be threaded "
        "into exactly one next use, not reused independently by two gates");
  }
  return mlir::success();
}
