/*
Copyright 2018 Brian Coopersmith

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

#include "scanner.h"
#include "tokens.h"

namespace pbc {
namespace {

void Scan(const std::string& code) {
  std::vector<std::unique_ptr<TokenPiece>> tokens;
  const ErrorCode ec = ScanTokens(code, tokens);
  if (ec.IsFailure()) {
    std::cerr << "Compilation failed.\n" << ec.ErrorMessage() << std::endl;
    return;
  }
  std::cout << "Compliation success!" << std::endl;
  for (const auto& token : tokens) {
    std::cout << "Token content: " << token->GetContent() << std::endl;
  }
}

}  // namespace
}  // namespace pbc

int main(int argc, char** argv) {
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
    pbc::Scan(code);
  }
}
