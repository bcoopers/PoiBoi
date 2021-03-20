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

// Fill out.
class RValueEvaluator {
 public:
  static ErrorOr<RValueEvaluator> TryCreate(const RValue& rv, CompilationContext& context);
  std::string GetCode() const;
 private:
  std::variant<QuotedString, Variable, std::unique_ptr<FunctionCallEvaluator>> op_;
};

ErrorOr<RValueEvaluator> RValueEvaluator::TryCreate(
    const RValue& va, CompilationContext& context) {
  return ErrorCode::Failure("RValueEvaluator unimplemented.");
}

std::string RValueEvaluator::GetCode() const {
  return "Unimplemented";
}

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
  return ErrorCode::Failure("VariableAssignmentEvaluator unimplemented.");
}

std::string VariableAssignmentEvaluator::GetCode() const {
  return "Unimplemented";
}

class BuiltinResolver {
  static ErrorOr<BuiltinResolver> TryCreate(const std::string& name);
  std::string GetCppName() const { return cpp_name_; }
 private:
  std::string cpp_name_;
};

ErrorOr<BuiltinResolver> BuiltinResolver::TryCreate(const std::string& name) {
  return ErrorCode::Failure("Unimplemented");
}

class FunctionCallEvaluator : public StatementEvaluator {
   public:
    static ErrorOr<FunctionCallEvaluator> TryCreate(const FunctionCall& fc, CompilationContext& context);
    std::string GetCode() const override;
   private:
    std::variant<std::string, BuiltinResolver> fn_name_or_builtin;
    std::vector<RValueEvaluator> args;
};

ErrorOr<FunctionCallEvaluator> FunctionCallEvaluator::TryCreate(
    const FunctionCall& fc, CompilationContext& context) {
  return ErrorCode::Failure("FunctionCallEvaluator unimplemented.");
}

std::string FunctionCallEvaluator::GetCode() const {
  return "Unimplemented";
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
