public class mierda {
	public static void main(String[] arg) {
		A a = new A();
		System.out.println(a.getFromA());
		B b = new B();
		System.out.println(b.getFromB());
		System.out.println(b.getFromA());
		System.exit(12);
	}
}

class A {
	static {
		System.out.println("A");
	}
	public int getFromA() { return 11; }
}

class B extends A {
	static {
		System.out.println("B");
	}
	public int getFromB() {
		return 14;
	}
}
