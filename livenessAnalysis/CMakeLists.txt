# add_llvm_pass_plugin(HW2Pass hw2pass.cpp)
find_package(LLVM 16 REQUIRED CONFIG)
add_llvm_pass_plugin(HW2Pass prac.cpp)