/*
Copyright 2021 Brian Coopersmith

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <fstream>
#include <iostream>
#include <streambuf>
#include <string>
#include <vector>

#include "codegen.h"
#include "parser.h"
#include "scanner.h"
#include "tokens.h"

namespace pbc {
namespace {

bool Scan(const std::string& code,
          std::vector<std::unique_ptr<TokenPiece>>& tokens) {
  const ErrorCode ec = ScanTokens(code, tokens);
  if (ec.IsFailure()) {
    std::cerr << "Compilation error.\n"
              << ec.ErrorMessage() << std::endl;
    return false;
  }
  return true;
}

bool Parse(std::vector<std::unique_ptr<TokenPiece>>& tokens,
           Module& module) {
  const ErrorCode ec = ParseTokens(tokens, module);
  if (ec.IsFailure()) {
    std::cerr << "Compilation error.\n"
              << ec.ErrorMessage() << std::endl;
    return false;
  }
  return true;
}

bool Generate(const std::vector<Module>& modules, std::string& code) {
  const ErrorCode ec = GenerateCode(modules, code);
  if (ec.IsFailure()) {
    std::cerr << "Compilation error.\n"
              << ec.ErrorMessage() << std::endl;
    return false;
  }
  return true;
}

}  // namespace
}  // namespace pbc

int main(int argc, char** argv) {
  std::vector<pbc::Module> roots;
  roots.reserve(argc - 1);
  for (int i = 1; i < argc; ++i) {
    const char* fname = argv[i];
    std::fstream filehandle;
    filehandle.open(fname, std::ios_base::in);
    if (!filehandle.is_open()) {
      std::cerr << "Compilation failed.\nCannot open file " << std::endl;
      return 1;
    }
    const std::string code{std::istreambuf_iterator<char>(filehandle),
                           std::istreambuf_iterator<char>()};
    filehandle.close();
    std::vector<std::unique_ptr<pbc::TokenPiece>> tokens;
    if (!pbc::Scan(code, tokens)) {
      std::cerr << "Compilation failed while scanning " << fname << std::endl;
      return 2;
    }
    for (auto& token : tokens) {
      token->set_file_name(fname);
    }
    roots.emplace_back();
    if (!pbc::Parse(tokens, roots.back())) {
      std::cerr << "Compilation failed while parsing " << fname << std::endl;
      return 3;
    }
  }
  std::string code;
  if (!pbc::Generate(roots, code)) {
    std::cerr << "Compilation failed in code generation." << std::endl;
    return 4;
  }
  std::cout << "Compilation successful!" << std::endl;
  return 0;
}
