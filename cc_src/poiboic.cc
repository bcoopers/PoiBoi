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
