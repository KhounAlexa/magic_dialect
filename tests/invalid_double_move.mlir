// Expected: FAIL -- the same created magic state is moved twice, forking
// one resource into two. A magic state resource is linear: it must be
// moved exactly once.
func.func @double_move() -> (!magic.state, !magic.state) {
  %m0 = "magic.create"() : () -> !magic.state
  %m1 = "magic.move"(%m0) : (!magic.state) -> !magic.state
  %m2 = "magic.move"(%m0) : (!magic.state) -> !magic.state
  return %m1, %m2 : !magic.state, !magic.state
}
