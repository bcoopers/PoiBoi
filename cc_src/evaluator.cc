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

#include "evaluator.h"

#include <variant>

#include "tokens.h"

namespace pbc {


class FunctionCallEvaluator;

struct VariableAccessor {
  bool is_local{};
  std::string name;
};

std::string LocalVariableName(const std::string& name) {
  return "LOCAL_VAR_" + name;
}

std::string GlobalVariableName(const std::string& name) {
  return "GLOBAL_VAR_" + name;
}

// Fill out.
class RValueEvaluator {
 public:
  static ErrorOr<RValueEvaluator> TryCreate(const RValue& rv, CompilationContext& context);
  std::string GetCode() const;
 private:
  RValueEvaluator(std::variant<QuotedString, VariableAccessor, std::unique_ptr<FunctionCallEvaluator>> op)
      : op_(std::move(op)) {}
  std::variant<QuotedString, VariableAccessor, std::unique_ptr<FunctionCallEvaluator>> op_;
};

// Fill out.
class VariableAssignmentEvaluator : public StatementEvaluator {
 public:
  static ErrorOr<VariableAssignmentEvaluator> TryCreate(const VariableAssignment& va, CompilationContext& context);
  std::string GetCode() const override;
 private:
  struct VarAssign {
    bool is_local = true;
    bool is_defined = true;
    std::string name;
    RValueEvaluator e;
  };
  std::vector<VarAssign> var_assigns_;
};

ErrorOr<VariableAssignmentEvaluator> VariableAssignmentEvaluator::TryCreate(
    const VariableAssignment& va, CompilationContext& context) {
  const auto& children = va.GetChildren();
  return ErrorCode::Failure("VariableAssignmentEvaluator unimplemented.");
}

std::string VariableAssignmentEvaluator::GetCode() const {
  return "Unimplemented";
}

enum class BuiltinType {
  EQUAL,
  PRINT,
  CONCAT,
  NOT,
  AND,
  OR,
  STRLEN,
  SUBSTRING,
};

class BuiltinResolver {
 public:
  static ErrorOr<BuiltinResolver> TryCreate(
      const std::string& name, size_t line_num, const std::string& fname);
  const std::string& GetCppName() const { return cpp_name_; }
  int GetNumArgs() const { return num_args_; }

 private:
  BuiltinResolver(BuiltinType type, std::string cpp_name, int num_args)
      : /*type_(type),*/ cpp_name_(std::move(cpp_name)), num_args_(num_args) {}
  // BuiltinType type_;
  std::string cpp_name_;
  int num_args_{};
};

ErrorOr<BuiltinResolver> BuiltinResolver::TryCreate(
    const std::string& name, size_t line_num, const std::string& fname) {
  if (name == "EQUAL") {
    return BuiltinResolver(BuiltinType::EQUAL, "Builtin_Equal", 2);
  } else if (name == "PRINT") {
    return BuiltinResolver(BuiltinType::PRINT, "Builtin_Print", 1);
  } else if (name == "CONCAT") {
    return BuiltinResolver(BuiltinType::CONCAT, "Builtin_Concat", 2);
  } else if (name == "NOT") {
    return BuiltinResolver(BuiltinType::NOT, "Builtin_Not", 1);
  } else if (name == "AND") {
    return BuiltinResolver(BuiltinType::AND, "Builtin_And", 2);
  } else if (name == "OR") {
    return BuiltinResolver(BuiltinType::OR, "Builtin_Or", 2);
  } else if (name == "STRLEN") {
    return BuiltinResolver(BuiltinType::STRLEN, "Builtin_Strlen", 1);
  } else if (name == "SUBSTRING") {
    return BuiltinResolver(BuiltinType::SUBSTRING, "Builtin_Substring", 3);
  }
  return ErrorCode::Failure("File: " + fname + "; line: " + std::to_string(line_num) +
                            "; Invalid builtin fn: " + name);
}

class FunctionCallEvaluator : public StatementEvaluator {
   public:
    static ErrorOr<FunctionCallEvaluator> TryCreate(const FunctionCall& fc, CompilationContext& context);
    std::string GetCode() const override;
   private:
    FunctionCallEvaluator(std::variant<std::string, BuiltinResolver> fnob, std::vector<RValueEvaluator> a) :
      fn_name_or_builtin_(std::move(fnob)), args_(std::move(a)) {}
    std::variant<std::string, BuiltinResolver> fn_name_or_builtin_;
    std::vector<RValueEvaluator> args_;
};

ErrorOr<RValueEvaluator> RValueEvaluator::TryCreate(
    const RValue& rv, CompilationContext& context) {
  const auto& children = rv.GetChildren();
  assert(children.size() == 1);
  const auto& child = *children[0];
  std::variant<QuotedString, VariableAccessor, std::unique_ptr<FunctionCallEvaluator>> op;
  if (child.GetLabel() == GrammarLabel::FUNCTION_CALL) {
    const FunctionCall& fc = dynamic_cast<const FunctionCall&>(child);
    auto fce = FunctionCallEvaluator::TryCreate(fc, context);
    RETURN_EC_IF_FAILURE(fce);
    op = std::make_unique<FunctionCallEvaluator>(std::move(fce.GetItem()));
  } else if (child.GetLabel() == GrammarLabel::QUOTED_STRING) {
    op = dynamic_cast<const QuotedString&>(child);
  } else {
    assert(child.GetLabel() == GrammarLabel::VARIABLE);
    const Variable& var = dynamic_cast<const Variable&>(child);
    const std::string& var_name = var.GetContent();
    if (context.curr_local_variables.count(var_name) == 1) {
      op = VariableAccessor{.is_local = true, .name = var_name};
    } else if (context.curr_global_variables.count(var_name) == 1) {
      op = VariableAccessor{.is_local = false, .name = var_name};
    } else {
      return ErrorCode::Failure("File: " + child.file_name() + "; line: " +
                                std::to_string(child.line_number()) +
                                "; Undefined variable: " + var_name);
    }
  }
  return RValueEvaluator(std::move(op));
}

std::string RValueEvaluator::GetCode() const {
  const QuotedString* quoted_string = std::get_if<QuotedString>(&op_);
  const VariableAccessor* variable = std::get_if<VariableAccessor>(&op_);
  const std::unique_ptr<FunctionCallEvaluator>* fn_call = std::get_if<std::unique_ptr<FunctionCallEvaluator>>(&op_);
  if (quoted_string != nullptr) {
    return quoted_string->GetContent();
  } else if (variable != nullptr) {
    return variable->is_local ? LocalVariableName(variable->name) : GlobalVariableName(variable->name);
  }
  assert(fn_call != nullptr);
  return (**fn_call).GetCode();

}

namespace {

std::vector<const RValue*> ExpandRValueList(const RValueList& rvl) {
  std::vector<const RValue*> rv_vec;
  const auto& rvl_children = rvl.GetChildren();
  if (rvl_children.size() == 0) {
    return rv_vec;
  }
  assert(rvl_children.size() == 2);
  assert(rvl_children[0]->GetLabel() == GrammarLabel::RVALUE);
  rv_vec.push_back(dynamic_cast<const RValue*>(&*rvl_children[0]));

  assert(rvl_children[1]->GetLabel() == GrammarLabel::RVALUE_LIST_EXPANSION);
  const RValueListExpansion* curr_expansion = dynamic_cast<const RValueListExpansion*>(&*rvl_children[1]);
  while (!curr_expansion->GetChildren().empty()) {
    const auto& expansion_children = curr_expansion->GetChildren();
    assert(expansion_children.size() == 3);
    assert(expansion_children[1]->GetLabel() == GrammarLabel::RVALUE);
    rv_vec.push_back(dynamic_cast<const RValue*>(&*expansion_children[1]));
    assert(expansion_children[2]->GetLabel() == GrammarLabel::RVALUE_LIST_EXPANSION);
    curr_expansion = dynamic_cast<const RValueListExpansion*>(&*expansion_children[2]);
  }
  return rv_vec;
}

}  // namespace


ErrorOr<FunctionCallEvaluator> FunctionCallEvaluator::TryCreate(
    const FunctionCall& fc, CompilationContext& context) {
  const auto& fc_children = fc.GetChildren();
  assert(fc_children.size() == 4);
  const auto& child0 = *fc_children[0];
  int num_args = 0;
  std::string debug_fn_name;
  std::variant<std::string, BuiltinResolver> fn_name_or_builtin{""};
  if (child0.GetLabel() == GrammarLabel::BUILTIN) {
    debug_fn_name = std::string("builtin ") + child0.GetContent();
    auto builtin = BuiltinResolver::TryCreate(
        child0.GetContent(), child0.line_number(), child0.file_name());
    RETURN_EC_IF_FAILURE(builtin);
    fn_name_or_builtin = std::move(builtin.GetItem());
    num_args = builtin.GetItem().GetNumArgs();
  } else {
    assert(child0.GetLabel() == GrammarLabel::FUNCTION_NAME);
    const std::string fn_name = child0.GetContent();
    debug_fn_name = std::string("function ") + fn_name;
    fn_name_or_builtin = fn_name;
    auto fn_it = context.fns->find(fn_name);
    if (fn_it == context.fns->end()) {
      return ErrorCode::Failure("File: " + child0.file_name() + "; line: " +
                                std::to_string(child0.line_number()) +
                                "; Tries to call undefined function: " + fn_name);
    }
    fn_name_or_builtin = fn_name;
    num_args = fn_it->second->GetVariablesList().size();
  }
  const auto& child2 = *fc_children[2];
  assert(child2.GetLabel() == GrammarLabel::RVALUE_LIST);

  std::vector<const RValue*> rvalue_args = ExpandRValueList(dynamic_cast<const RValueList&>(child2));
  if (rvalue_args.size() != num_args) {
    return ErrorCode::Failure("File: " + child0.file_name() + "; line: " +
                              std::to_string(child0.line_number()) +
                              "; Tries to call " + debug_fn_name + " with " +
                              std::to_string(rvalue_args.size()) + " args, but expected " +
                              std::to_string(num_args) + "args");
  }
  std::vector<RValueEvaluator> args;
  args.reserve(num_args);
  for (const RValue* rval : rvalue_args) {
    auto rvalue_eval = RValueEvaluator::TryCreate(*rval, context);
    RETURN_EC_IF_FAILURE(rvalue_eval);
    args.push_back(std::move(rvalue_eval.GetItem()));
  }
  return FunctionCallEvaluator(std::move(fn_name_or_builtin), std::move(args));
}

std::string FunctionCallEvaluator::GetCode() const {
  std::string code;
  const std::string* fn_name = std::get_if<std::string>(&fn_name_or_builtin_);
  if (fn_name != nullptr) {
    code += *fn_name + "(";
  } else {
    const BuiltinResolver* builtin = std::get_if<BuiltinResolver>(&fn_name_or_builtin_);
    assert(builtin != nullptr);
    code += builtin->GetCppName() + "(";
  }
  for (int i = 0; i < args_.size(); ++i) {
    code += args_.at(i).GetCode();
    if (i + 1 != args_.size()) {
      code += ", ";
    }
  }
  code += ");";
  return code;
}

class WhileEvaluator : public StatementEvaluator {
 public:
  static ErrorOr<WhileEvaluator> TryCreate(const ConditionalEvaluation& ce, const CodeBlock& cb,
                                           CompilationContext& context);
  std::string GetCode() const override;
 private:
  RValueEvaluator conditional_;
  CodeBlockEvaluator cbe_;
  WhileEvaluator(RValueEvaluator cond, CodeBlockEvaluator cbe) :
      conditional_(std::move(cond)), cbe_(std::move(cbe)) {}
};

ErrorOr<WhileEvaluator> WhileEvaluator::TryCreate(
    const ConditionalEvaluation& ce, const CodeBlock& cb, CompilationContext& context) {
  const auto& ce_children = ce.GetChildren();
  assert(ce_children.size() == 3);
  auto conditional = RValueEvaluator::TryCreate(
      dynamic_cast<const RValue&>(*ce_children[1]), context);
  RETURN_EC_IF_FAILURE(conditional);
  CompilationContext inside_loop_context = context;
  inside_loop_context.is_in_loop = true;
  auto code = CodeBlockEvaluator::TryCreate(cb, std::move(inside_loop_context));
  RETURN_EC_IF_FAILURE(code);
  return WhileEvaluator(std::move(conditional.GetItem()), std::move(code.GetItem()));
}

std::string WhileEvaluator::GetCode() const {
  std::string code = "while (" + conditional_.GetCode() + ") {\n";
  code += cbe_.GetCode();
  code += "}\n";
  return code;
}

class IfEvaluator : public StatementEvaluator {
 public:
  static ErrorOr<IfEvaluator> TryCreate(const ConditionalEvaluation& ce, const CodeBlock& cb,
                                        const ElseStatement& ee, CompilationContext& context);
  std::string GetCode() const override;
 private:
  struct IfOrElse {
    std::optional<RValueEvaluator> maybe_conditional;
    CodeBlockEvaluator cbe;
  };
  std::vector<IfOrElse> ifs_and_elses_;
  IfEvaluator(std::vector<IfOrElse> ioes) : ifs_and_elses_(std::move(ioes)) {}
};

ErrorOr<IfEvaluator> IfEvaluator::TryCreate(const ConditionalEvaluation& ce, const CodeBlock& cb,
                                            const ElseStatement& ee, CompilationContext& context) {
  std::vector<IfOrElse> ifs_and_elses;
  const auto& ce_children = ce.GetChildren();
  assert(ce_children.size() == 3);
  auto conditional = RValueEvaluator::TryCreate(
      dynamic_cast<const RValue&>(*ce_children[1]), context);
  RETURN_EC_IF_FAILURE(conditional);
  auto code = CodeBlockEvaluator::TryCreate(cb, context);
  RETURN_EC_IF_FAILURE(code);
  ifs_and_elses.emplace_back(IfOrElse{
      std::move(conditional.GetItem()), std::move(code.GetItem())});
  const auto* else_children = &ee.GetChildren();
  while (else_children->size() != 0) {
    if (else_children->size() == 2) {
      // ELSE statement.
      auto elsecode = CodeBlockEvaluator::TryCreate(
          dynamic_cast<const CodeBlock&>(*else_children->at(1)), context);
      RETURN_EC_IF_FAILURE(elsecode);
      ifs_and_elses.emplace_back(IfOrElse{std::nullopt, std::move(elsecode.GetItem())});
      break;
    }
    // ELIF statement.
    assert(else_children->size() == 4);
    const auto& additional_conditional = dynamic_cast<const ConditionalEvaluation&>(
        *else_children->at(1));
    assert(additional_conditional.GetChildren().size() == 2);
    auto else_conditional = RValueEvaluator::TryCreate(
        dynamic_cast<const RValue&>(*additional_conditional.GetChildren()[1]), context);
    RETURN_EC_IF_FAILURE(else_conditional);
    auto else_code = CodeBlockEvaluator::TryCreate(
        dynamic_cast<const CodeBlock&>(*else_children->at(2)), context);
    RETURN_EC_IF_FAILURE(else_code);
    ifs_and_elses.emplace_back(IfOrElse{
        std::move(else_conditional.GetItem()), std::move(else_code.GetItem())});
    const auto& additonal_else = dynamic_cast<const ElseStatement&>(*else_children->at(3));
    else_children = &additonal_else.GetChildren();
  }
  return IfEvaluator(std::move(ifs_and_elses));
}

std::string IfEvaluator::GetCode() const {
  std::string code = "if (" + ifs_and_elses_.at(0).maybe_conditional.value().GetCode() + ") {\n";
  code += ifs_and_elses_.at(0).cbe.GetCode();
  code += "}";
  for (int i = 1; i < ifs_and_elses_.size(); ++i) {
    const auto& iae = ifs_and_elses_.at(i);
    if (iae.maybe_conditional.has_value()) {
      code += " else if (" + iae.maybe_conditional.value().GetCode() + ") {\n";
    } else {
      code += " else {\n";
    }
    code += iae.cbe.GetCode();
    code += "}";
  }
  code += "\n";
  return code;
}

class ReturnEvaluator : public StatementEvaluator {
 public:
  static ErrorOr<ReturnEvaluator> TryCreate(const RValue& rvalue, CompilationContext& context);
  std::string GetCode() const override;
 private:
  ReturnEvaluator(RValueEvaluator rve) : rve_(std::move(rve)) {}
  RValueEvaluator rve_;
};

ErrorOr<ReturnEvaluator> ReturnEvaluator::TryCreate(
    const RValue& rvalue, CompilationContext& context) {
  auto rve = RValueEvaluator::TryCreate(rvalue, context);
  RETURN_EC_IF_FAILURE(rve);
  return ReturnEvaluator(std::move(rve.GetItem()));
}

std::string ReturnEvaluator::GetCode() const {
  return "return " + rve_.GetCode() + ";\n";
}

class BreakEvaluator : public StatementEvaluator {
 public:
  static ErrorOr<BreakEvaluator> TryCreate(
      size_t line_num, std::string file, CompilationContext& context);
  std::string GetCode() const override;
 private:
  BreakEvaluator() {}
};

ErrorOr<BreakEvaluator> BreakEvaluator::TryCreate(
    size_t line_num, std::string file, CompilationContext& context) {
  if (!context.is_in_loop) {
    return ErrorCode::Failure("File: " + file + "; line: " + std::to_string(line_num) +
                              "; Cannot have a BREAK outside of a loop.");
  }
  return BreakEvaluator();
}

std::string BreakEvaluator::GetCode() const {
  return "break;\n";
}

ErrorOr<std::unique_ptr<StatementEvaluator>> StatementEvaluator::TryCreate(
    const Statement& statement, CompilationContext& context) {
  using ReturnVal = std::unique_ptr<StatementEvaluator>;
  const auto& children = statement.GetChildren();
  assert(children.size() > 0);
  auto label = children[0]->GetLabel();
  if (label == GrammarLabel::VARIABLE_ASSIGNMENT) {
    auto eval = VariableAssignmentEvaluator::TryCreate(
        dynamic_cast<const VariableAssignment&>(*children[0]), context);
    RETURN_EC_IF_FAILURE(eval);
    return ReturnVal(std::make_unique<VariableAssignmentEvaluator>(std::move(eval.GetItem())));
  } else if (label == GrammarLabel::FUNCTION_CALL) {
    auto eval = FunctionCallEvaluator::TryCreate(
        dynamic_cast<const FunctionCall&>(*children[0]), context);
    RETURN_EC_IF_FAILURE(eval);
    return ReturnVal(std::make_unique<FunctionCallEvaluator>(std::move(eval.GetItem())));
  } else if (label == GrammarLabel::KEYWORD_WHILE) {
    assert(children.size() == 3);
    auto eval = WhileEvaluator::TryCreate(
        dynamic_cast<const ConditionalEvaluation&>(*children[1]),
        dynamic_cast<const CodeBlock&>(*children[2]), context);
    RETURN_EC_IF_FAILURE(eval);
    return ReturnVal(std::make_unique<WhileEvaluator>(std::move(eval.GetItem())));
  } else if (label == GrammarLabel::KEYWORD_IF) {
    assert(children.size() == 4);
    auto eval = IfEvaluator::TryCreate(
        dynamic_cast<const ConditionalEvaluation&>(*children[1]),
        dynamic_cast<const CodeBlock&>(*children[2]),
        dynamic_cast<const ElseStatement&>(*children[3]), context);
    RETURN_EC_IF_FAILURE(eval);
    return ReturnVal(std::make_unique<IfEvaluator>(std::move(eval.GetItem())));
  } else if (label == GrammarLabel::KEYWORD_RETURN) {
    assert(children.size() == 3);
    auto eval = ReturnEvaluator::TryCreate(
        dynamic_cast<const RValue&>(*children[1]), context);
    RETURN_EC_IF_FAILURE(eval);
    return ReturnVal(std::make_unique<ReturnEvaluator>(std::move(eval.GetItem())));
  } else if (label == GrammarLabel::KEYWORD_BREAK) {
    auto eval = BreakEvaluator::TryCreate(
        children[0]->line_number(), children[0]->file_name(), context);
    RETURN_EC_IF_FAILURE(eval);
    return ReturnVal(std::make_unique<BreakEvaluator>(std::move(eval.GetItem())));
  }
  assert(false);
}

ErrorOr<CodeBlockEvaluator> CodeBlockEvaluator::TryCreate(
    const CodeBlock& code_block, CompilationContext context) {
  std::vector<std::unique_ptr<StatementEvaluator>> evaluators;
  assert(code_block.GetChildren().size() == 3);
  const StatementList* statement_list = &dynamic_cast<const StatementList&>(
    *code_block.GetChildren()[1]);
  while (statement_list->GetChildren().size() != 0) {
    const auto& children = statement_list->GetChildren();
    assert(children.size() == 2);
    auto statement_evaluator = StatementEvaluator::TryCreate(
      dynamic_cast<const Statement&>(*children[0]), context);
    RETURN_EC_IF_FAILURE(statement_evaluator);
    evaluators.push_back(std::move(statement_evaluator.GetItem()));
    statement_list = &dynamic_cast<const StatementList&>(*children[1]);
  }
  return CodeBlockEvaluator(std::move(evaluators));
}

std::string CodeBlockEvaluator::GetCode() const {
  std::string code;
  for (const auto& evaluator : evaluators_) {
    code += evaluator->GetCode();
  }
  return code;
}

}  // namespace pbc
