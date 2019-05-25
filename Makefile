comp: lex.yy.c binary.tab.o Utils.cpp ThreeAdd.cpp BasicBlock.cpp Node.cpp main.cpp labTrees.cc
		g++ -std=gnu++11 -g -o comp binary.tab.o lex.yy.c Utils.cpp ThreeAdd.cpp BasicBlock.cpp Node.cpp main.cpp labTrees.cc
		rm -f binary.tab.* lex.yy.c*
binary.tab.o: binary.tab.cc
		g++ -std=gnu++11 -g -c binary.tab.cc
binary.tab.cc: binary.yy
		bison binary.yy
lex.yy.c: binary.ll binary.tab.cc
		flex binary.ll
run: target.exe
	./target
target.exe: target.cc
	g++ -std=gnu++11 -g -o target target.cc
tar: Utils.* ThreeAdd.* BasicBlock.* Node.* main.cpp labTrees.cc binary.* Makefile
	tar czf submission.tgz --transform 's,^,ass2-comp/,' Utils.* ThreeAdd.* BasicBlock.* Node.* main.cpp labTrees.cc binary.* Makefile
clean: 
		rm -f binary.tab.* lex.yy.c* comp *.dot cfg.dot.svg target.cc target.exe *.o *.stackdump submission.tgz
