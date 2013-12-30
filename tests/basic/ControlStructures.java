package basic;
/*
 * Control structures
 */
public class ControlStructures {
	public static int test0() {
		/*
		 * Description:
		 * for
		 */
		int c = 0;
		for (int i = 0 ; i < 100 ; i++) c++;

		for (int i = 200 ; i > -100 ; i--) c -= i;

		for (int i = 123 ; i < 10000 && i % 211 != 0 ; i++) c++;

		for (int i = 0 ; i < 10000 ; i++) {
			if (i % 111 == 0) break;
			c++;
		}

		for (int i = 0 ; i < 4500 ; i++) {
			if (i % 2 == 0)
				continue;
			c++;
		}

		return c;
	}

	public static int test1() {
		/*
		 * Description:
		 * switch
		 */
	        int c0 = 0;
		int c1 = 0;
		int c2 = 0;
		for (int i = 0 ; i < 12222 ; i++) {
			int r = i % 5;
			switch (r) {
				case 0:
					c0 ++;
				break;
				case 1:
					c1 ++;
				break;
				default:
					c2 ++;
				break;
			}
		}
		return c0 + c1 + c2;
	}

	public static int test2() {
		/*
		 * Description:
		 * if-else
		 */
		int r = 12000;
		if (r > 1300000)
			return 13;
		else
			r = 134;
		return r;
	}
}
