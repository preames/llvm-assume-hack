clang++ -S -emit-llvm -x c++ -fPIC -std=c++0x -O3 assume.c3x -o build/assume.ll
../build/Release/bin/opt -preverify -verify-each -load ../build/Release/lib/LLVMDropAssumptionsPass.so -drop-assumptions build/assume.ll -S -o build/assume-prune.ll
clang++ -S -emit-llvm -fPIC -O3 build/assume-prune.ll -o build/assume-opt.ll
