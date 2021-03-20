#ifndef POIBOIC_CODEGEN_H_
#define POIBOIC_CODEGEN_H_

#include "error_code.h"
#include "grammar.h"

namespace pbc {

ErrorCode GenerateCode(const std::vector<Module>& modules, std::string& code_out);

}  // namespace pbc

#endif  // #ifndef POIBOIC_CODEGEN_H_
