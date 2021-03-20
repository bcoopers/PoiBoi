#include "codegen.h"

#include "function.h"


namespace pbc {

ErrorCode GenerateCode(const std::vector<Module>& modules, std::string& code_out) {
  std::vector<Function> functions = GetFunctionsFromModules(modules);
  std::unordered_map<std::string, const Function*> functions_dict = GetFunctionsDict(functions);

/*
  for (const Function& fn : functions) {
    std::cout << "Function: " << fn.GetName() << std::endl;
    for (const auto& var : fn.GetVariablesList()) {
      std::cout << "  " << var << std::endl;
    }
  }
*/

  return ErrorCode::Success();
}

}  // namespace pbc
