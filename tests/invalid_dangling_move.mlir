// Expected: FAIL -- a magic state is moved and then escapes (here, via
// return) without ever being consumed. A moved magic state must reach a
// 'magic.consume' and cannot escape the create/move/consume chain.
func.func @dangling_move() -> !magic.state {
  %m0 = "magic.create"() : () -> !magic.state
  %m1 = "magic.move"(%m0) : (!magic.state) -> !magic.state
  return %m1 : !magic.state
}
