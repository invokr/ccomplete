{
  "targets": [
    {
      "target_name": "ccomplete",
      "sources": ["src/ccomplete.cpp"],
      "cflags_cc": ["-g", "-std=c++0x", "-Wall", "-I/usr/local/llvm35/include/clang/"],
      "libraries": ["-lclang", "-lLLVM-3.5", "-L/usr/local/llvm35/lib/"]
    }
  ]
}


