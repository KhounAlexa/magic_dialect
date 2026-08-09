// Expected: FAIL -- move's operand is a plain function argument,
// not something produced by magic.create.
func.func @invalid_move(%not_magic: !magic.state) -> !magic.state {
  %m1 = "magic.move"(%not_magic) : (!magic.state) -> !magic.state
  return %m1 : !magic.state
}