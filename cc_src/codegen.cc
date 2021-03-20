#include "codegen.h"

#include <string>
#include <fstream>
#include <streambuf>

#include "function.h"
#include "interpretation_context.h"

namespace pbc {
namespace {
constexpr char kPbStringType[] = "PBString ";
constexpr char kFnSuffix[] = "_poiboi_fn";
constexpr char kLocalVarSuffix[] = "_local_poiboivar";

void AddPBStringSrc(std::string& code_out) {
  std::ifstream fh("cc_src/poiboi_string.h");
  code_out.insert(code_out.end(), (std::istreambuf_iterator<char>(fh)),
                  std::istreambuf_iterator<char>());
  std::ifstream fc("cc_src/poiboi_string.cc");
  code_out.insert(code_out.end(), (std::istreambuf_iterator<char>(fc)),
                  std::istreambuf_iterator<char>());

}

std::string GetFunctionDeclaration(const Function& fn) {
  std::string code = kPbStringType + fn.GetName() + kFnSuffix + "(";
  if (fn.GetVariablesList().size() > 0) {
    code += kPbStringType + fn.GetVariablesList()[0] + kLocalVarSuffix;
    for (int i = 1; i < fn.GetVariablesList().size(); ++i) {
      code += ", " + (kPbStringType + fn.GetVariablesList()[i]) + kLocalVarSuffix;
    }
  }
  code += ")";
  return code;
}

ErrorCode GetFunctionDefinition(const Function& fn, CompilationContext& context,
                                std::string& code) {
  code = GetFunctionDeclaration(fn) + "{\n";

  // TODO: Flesh this out.
  code += "}\n\n";
  return ErrorCode::Success();
}
}  // namespace

ErrorCode GenerateCode(const std::vector<Module>& modules, std::string& code_out) {
  code_out.clear();
  std::vector<Function> functions = GetFunctionsFromModules(modules);
  if (functions.empty()) {
    return ErrorCode::Success();
  }
  std::unordered_map<std::string, const Function*> functions_dict = GetFunctionsDict(functions);
  std::vector<std::string> global_variables;

  code_out += "#define POIBOI_EXECUTABLE_\n#define POIBOI_INCLUDE_ASSERT_\n";
  AddPBStringSrc(code_out);

  for (const Function& fn : functions) {
    code_out += GetFunctionDeclaration(fn) + ";\n";
  }

  CompilationContext context{.fns = &functions_dict, .global_variables = &global_variables};

  std::vector<std::string> fn_definitions;
  for (const Function& fn : functions) {
    const ErrorCode code = GetFunctionDefinition(fn, context, fn_definitions.emplace_back());
    if (code.IsFailure()) {
      return code;
    }
  }

  // TODO: Add global variable, fn definitions, and main.

  return ErrorCode::Success();
}

}  // namespace pbc
