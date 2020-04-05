Implementation of a LUA compiler using BISON and flex, which compiles Lua to a target.s. 
The target can then be compiled with gcc, the target.s file is linked with libc for I/O and exponentation functionality.

The testcase folder contains lua samples with supported lua functionality, the compiler
can not handle all of LUA syntax:

Supported syntax:
* Constant expressions (testcase1.lua)
* Assignment statements (testcase2.lua)
* I/O and control-flow (testcase3.lua & testcase4.lua)
* Tables (testcase5.lua)
* Recursive Functions (testcase6.lua)

To use the compiler clone project:
1. Write LUA code for compilation in Testcases/exeriment.lua
2. Type in Linux terminal: "Make comp" - Compiles compiler
3. Execute compiler: "./comp <lua file>" creates target.s assebly file of compiled LUA code
4. Make run to compile target.s with gcc 
5. Compiled executable should now exist called a
6. Execute compilations by typing "./a" 

