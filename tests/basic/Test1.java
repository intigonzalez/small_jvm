package basic;
/*
 * This file has tests for integer arithmetic.
 */
public class Test1 {
	
	// ====================================
	// Accesing constants
	// ====================================
	public static int test0() {
		return 13;
	}

	public static int test1() {
		return 123354556;
	}

	public static int test2() {
		return 38908;
	}

	// ====================================
	// Simple operations
	// ====================================
	public static int test3() {
		int a = 123;
		int b = 56789;
		return a + b;
	}
	
	public static int test4() {
		int a = 123;
		int b = 56789;
		return a * b;
	}

	public static int test5() {
		int a = 123;
		int b = 56789;
		return b / a;
	}

	public static int test6() {
		int a = 123;
		int b = 56789;
		return b % a;
	}

	// =====================================
	// Several operations
	// =====================================
	public static int test7() {
		int a = 123;
		int b = 56789;
		return (a + b) * 3 + (a - 2) / 14;
	}

	public static int test8() {
		int a = 123;
		int b = 567;
		return (a*a + b*b)/(2*a);
	}

	public static int test9() {
		int a = 567;
		int b = a << 1;
		int c = b >> 2;
		int d = a + b + c;
		a = b = c = d;
		return a + b + c + d % 134;
	}
}
