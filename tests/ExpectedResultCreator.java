import java.util.*;
import java.lang.reflect.*;

public class ExpectedResultCreator {

	public static void main(String[] arg) throws Exception {
		String className = arg[0];
		
		Class<?> clazz = Class.forName(className);

		// iterate over every method
		Method[] methods = clazz.getDeclaredMethods();

		ArrayList<Method> l = new ArrayList<Method>();
		for (int i = 0 ; i < methods.length ; i++) {
			if (methods[i].getName().startsWith("test") && 
					Modifier.isStatic(methods[i].getModifiers()))
				l.add(methods[i]);
		}

		// sort methods by name
		Collections.sort(l, new Comparator<Method>() {
			public int compare(Method m1, Method m2) {
				String s1 = m1.getName().substring(4);
				String s2 = m2.getName().substring(4);
				int i1 = Integer.parseInt(s1);
				int i2 = Integer.parseInt(s2);
				if (i1 < i2) return -1;
				if (i1 == i2) return 0;
				return 1;
			}
		});

		if (arg[1].equals("-l")) {
			for (Method m : l) 
				System.out.println(m.getName());
		}
		else {
			for (Method m : l)
				if (m.getName().equals(arg[1]))
					System.out.println(m.invoke(null));
		}
	}
}

