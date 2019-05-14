comp: labUtils.cpp labTrees.cc
		g++ -std=gnu++11 -g -o comp labTrees.cc labUtils.cpp
clean: 
		rm -f comp cfg.dot cfg.dot.svg 
