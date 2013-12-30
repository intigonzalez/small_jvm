package basic;
/*
 * Use some classes from JDK to create objects. Very basic, without pushing the collector.
 */

import java.util.*;

public class instances1 {

	public static int test0() {
		/* 
		 * Description: 
		 * Create object of class Integer
		 */
	       	Integer e = 123;
		Integer x = e + 145;	
		return x;
	}

	public static int test1() {
		/* 
		 * Description: 
		 * Create ArrayList of Integer
		 */
	       	ArrayList<Integer> l = new ArrayList<Integer>();
		for (int i = 0 ; i < 30 ; i++)
			l.add(i);
		return l.get(15);
	}

	public static int test2() {
		/* Description 
		 * LinkedList
		 */
	       	LinkedList<Integer> l = new LinkedList<Integer>();
		for (int i = 0 ; i < 100 ; i++)
			l.add(i);
			
		return l.size();
	}
}

