#ifndef POIBOIC_FUNCTION_H_
#define POIBOIC_FUNCTION_H_

#include <string>
#include <unordered_map>
#include <vector>

#include "grammar.h"

namespace pbc {

class Function {
 public:
  Function(const FunctionDefinition& fn_def);
  const std::string& GetName() const { return name_; }
  const std::vector<std::string>& GetVariablesList() const { return variables_list_; }
  const CodeBlock& GetCode() const { return *code_; }



 private:
  std::string name_;
  std::vector<std::string> variables_list_;
  const CodeBlock* code_{};

  std::string file_name_;
  size_t line_num_{};
};

std::vector<Function> GetFunctionsFromModules(const std::vector<Module>& modules);

std::unordered_map<std::string, const Function*> GetFunctionsDict(const std::vector<Function>& fns_list);

}  // namespace pbc

#endif  // #ifndef POIBOIC_FUNCTION_H_
