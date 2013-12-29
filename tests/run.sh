#!/bin/bash

# first we must compile the tests
javac */*.java

# compile the helper program
javac ExpectedResultCreator.java

# iterate over directories
for gtc in * ; do
	if [ -d "$gtc" ] ; then
		echo "Testing cases within $gtc"

		# iterate over test cases (is a class with many tests inside)
		for tc_file in $gtc/*.class ; do
			tcOld="${tc_file/%.class}"
			tc="${tcOld/\//.}"

			e=`mktemp`
			o=`mktemp`
			# iterate over test (needs the help of java) 
			for t in `java ExpectedResultCreator $tc -l` ; do
				java ExpectedResultCreator "$tc" $t > "$e"
				../Debug/vm2 -t $t $tc /home/inti/programs/cp99/lib/ > $o
				if diff $e $o > /dev/null ; then
					echo -e "${tc}:$t -> \e[1;32mSUCCESS\e[0m"
				else
					echo -e "${tc}:$t -> \e[1;31mFAIL\e[0m"
				fi
			done
			rm $e $o
			rm *.bin *.asm	
		done
	fi
done
