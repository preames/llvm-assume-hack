set -x
set -e
mkdir -p ./output
clang++ -O3 -S -emit-llvm assume.cpp -o ./output/assume.ll
$LLVM_DIR./bin/opt -load $LLVM_DIR./lib/LLVMDropAssumptions.so -drop-assumptions ./output/assume.ll -S -o ./output/assume-prune.ll
$LLVM_DIR./bin/opt -S -preverify -O3 ./output/assume-prune.ll -o ./output/assume-opt.ll
