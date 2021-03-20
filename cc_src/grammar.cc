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

#include "grammar.h"

#include "tokens.h"

namespace pbc {

GrammarPiece::Descendents Module::GetDescendents() const {
  Descendents d;

  d.emplace_back();
  d.back().emplace_back(new EndOfFile);

  d.emplace_back();
  d.back().emplace_back(new FunctionDefinition);
  d.back().emplace_back(new Module);

  return d;
}

GrammarPiece::Descendents FunctionDefinition::GetDescendents() const {
  Descendents d;

  d.emplace_back();
  d.back().emplace_back(new FunctionName);
  d.back().emplace_back(new OpenFunctionCall);
  d.back().emplace_back(new VariablesList);
  d.back().emplace_back(new CloseFunctionCall);
  d.back().emplace_back(new CodeBlock);

  return d;
}

GrammarPiece::Descendents VariablesList::GetDescendents() const {
  Descendents d;

  d.emplace_back();
  d.back().emplace_back(new Variable);
  d.back().emplace_back(new VariablesListExpansion);

  d.emplace_back();

  return d;
}

GrammarPiece::Descendents VariablesListExpansion::GetDescendents() const {
  Descendents d;

  d.emplace_back();
  d.back().emplace_back(new ArgumentListSeparator);
  d.back().emplace_back(new Variable);
  d.back().emplace_back(new VariablesListExpansion);

  d.emplace_back();

  return d;
}

GrammarPiece::Descendents CodeBlock::GetDescendents() const {
  Descendents d;

  d.emplace_back();
  d.back().emplace_back(new OpenCodeBlock);
  d.back().emplace_back(new StatementList);
  d.back().emplace_back(new CloseCodeBlock);

  return d;
}

GrammarPiece::Descendents StatementList::GetDescendents() const {
  Descendents d;

  d.emplace_back();
  d.back().emplace_back(new Statement);
  d.back().emplace_back(new StatementList);

  d.emplace_back();

  return d;
}

GrammarPiece::Descendents Statement::GetDescendents() const {
  Descendents d;

  d.emplace_back();
  d.back().emplace_back(new VariableAssignment);
  d.back().emplace_back(new EndStatement);

  d.emplace_back();
  d.back().emplace_back(new FunctionCall);
  d.back().emplace_back(new EndStatement);

  d.emplace_back();
  d.back().emplace_back(new KeywordWhile);
  d.back().emplace_back(new ConditionalEvaluation);
  d.back().emplace_back(new CodeBlock);

  d.emplace_back();
  d.back().emplace_back(new KeywordIf);
  d.back().emplace_back(new ConditionalEvaluation);
  d.back().emplace_back(new CodeBlock);
  d.back().emplace_back(new ElseStatement);

  d.emplace_back();
  d.back().emplace_back(new KeywordReturn);
  d.back().emplace_back(new RValue);
  d.back().emplace_back(new EndStatement);

  d.emplace_back();
  d.back().emplace_back(new KeywordBreak);
  d.back().emplace_back(new EndStatement);

  return d;
}

GrammarPiece::Descendents VariableAssignment::GetDescendents() const {
  Descendents d;

  d.emplace_back();
  d.back().emplace_back(new KeywordLocal);
  d.back().emplace_back(new Variable);
  d.back().emplace_back(new Assigner);
  d.back().emplace_back(new RValue);

  d.emplace_back();
  d.back().emplace_back(new KeywordGlobal);
  d.back().emplace_back(new Variable);
  d.back().emplace_back(new Assigner);
  d.back().emplace_back(new RValue);

  d.emplace_back();
  d.back().emplace_back(new Variable);
  d.back().emplace_back(new Assigner);
  d.back().emplace_back(new RValue);

  return d;
}

GrammarPiece::Descendents FunctionCall::GetDescendents() const {
  Descendents d;

  d.emplace_back();
  d.back().emplace_back(new FunctionName);
  d.back().emplace_back(new OpenFunctionCall);
  d.back().emplace_back(new RValueList);
  d.back().emplace_back(new CloseFunctionCall);

  d.emplace_back();
  d.back().emplace_back(new Builtin);
  d.back().emplace_back(new OpenFunctionCall);
  d.back().emplace_back(new RValueList);
  d.back().emplace_back(new CloseFunctionCall);

  return d;
}

GrammarPiece::Descendents ConditionalEvaluation::GetDescendents() const {
  Descendents d;

  d.emplace_back();
  d.back().emplace_back(new OpenConditionalBlock);
  d.back().emplace_back(new RValue);
  d.back().emplace_back(new CloseConditionalBlock);

  return d;
}

GrammarPiece::Descendents ElseStatement::GetDescendents() const {
  Descendents d;

  d.emplace_back();
  d.back().emplace_back(new KeywordElse);
  d.back().emplace_back(new CodeBlock);

  d.emplace_back();
  d.back().emplace_back(new KeywordElif);
  d.back().emplace_back(new ConditionalEvaluation);
  d.back().emplace_back(new CodeBlock);
  d.back().emplace_back(new ElseStatement);

  d.emplace_back();

  return d;
}

GrammarPiece::Descendents RValue::GetDescendents() const {
  Descendents d;

  d.emplace_back();
  d.back().emplace_back(new QuotedString);

  d.emplace_back();
  d.back().emplace_back(new Variable);

  d.emplace_back();
  d.back().emplace_back(new FunctionCall);

  return d;
}

GrammarPiece::Descendents RValueList::GetDescendents() const {
  Descendents d;

  d.emplace_back();
  d.back().emplace_back(new RValue);
  d.back().emplace_back(new RValueListExpansion);

  d.emplace_back();

  return d;
}

GrammarPiece::Descendents RValueListExpansion::GetDescendents() const {
  Descendents d;

  d.emplace_back();
  d.back().emplace_back(new ArgumentListSeparator);
  d.back().emplace_back(new RValue);
  d.back().emplace_back(new RValueListExpansion);

  d.emplace_back();

  return d;
}
}  // namespace pbc
