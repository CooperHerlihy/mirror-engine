#include <print>

using std::println;

struct Movable {
	Movable() {
		println("constructed");
	}
	~Movable() noexcept {
		println("destructed");
	}
	Movable(const Movable&) {
		println("copy constructed");
	}
	Movable& operator=(const Movable&) {
		println("copy assigned");
	}
	Movable(Movable&&) noexcept {
		println("move constructed");
	}
	Movable& operator=(Movable&&) noexcept {
		println("move assigned");
	}
};

struct UsingMovable {
	Movable m1;
	Movable m2;
};

int main() {
	UsingMovable a;
	UsingMovable b = std::move(a);
}
