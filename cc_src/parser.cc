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

#include "parser.h"

namespace pbc {
namespace {

std::string ErrorString(const GrammarPiece& gp, const TokenPiece& token) {
  return "Line " + std::to_string(token.line_number() + 1) +
         ": Parsing error.\nExpected: " + gp.DebugDescription() +
         "\nGot: " + token.GetContent();
}

bool Expand(const GrammarPiece& gp, const TokenPiece& token,
            std::vector<std::unique_ptr<GrammarPiece>>& expansion) {
  expansion.clear();
  if (gp.GetLabel() == token.GetLabel()) {
    expansion.push_back(token.Clone());
    return true;
  } else if (gp.IsToken()) {
    return false;
  }
  bool allow_empty_expansion = false;
  GrammarPiece::Descendents descendents = gp.GetDescendents();
  for (auto& descendent_list : descendents) {
    if (descendent_list.empty()) {
      allow_empty_expansion = true;
    } else {
      std::vector<std::unique_ptr<GrammarPiece>> placeholder;
      if (Expand(*descendent_list.front(), token, placeholder)) {
        assert(expansion.empty());  // Should not have ambiguous grammar.
        expansion = std::move(descendent_list);
      }
    }
  }
  for (auto& piece : expansion) {
    piece->set_line_number(token.line_number());
    piece->set_file_name(token.file_name());
  }
  return allow_empty_expansion || !expansion.empty();
}
}  // namespace

ErrorCode ParseTokens(std::vector<std::unique_ptr<TokenPiece>>& tokens,
                      Module& root) {
  assert(tokens.size() > 0);
  root = Module();
  root.set_line_number(0);
  root.set_file_name(tokens.front()->file_name());
  std::vector<GrammarPiece*> current_program;
  current_program.emplace_back(&root);
  size_t token_index = 0;
  while (current_program.size() > 0) {
    auto& gp = *current_program.front();
    auto& token = *tokens.at(token_index);
    std::vector<std::unique_ptr<GrammarPiece>> expansion;
    if (!Expand(gp, token, expansion)) {
      return ErrorCode::Failure(ErrorString(gp, token));
    }
    if (gp.IsToken()) {
      TokenPiece& program_front = reinterpret_cast<TokenPiece&>(*current_program.front());
      token.CopyTokenTo(&program_front);
      ++token_index;
      current_program.erase(current_program.begin());
    } else {
      std::vector<GrammarPiece*> expansion_ptrs;
      for (const auto& expanded_piece : expansion) {
        expansion_ptrs.push_back(expanded_piece.get());
      }
      current_program.front()->MoveIntoChildren(expansion);
      current_program.erase(current_program.begin());
      current_program.insert(current_program.begin(), expansion_ptrs.begin(),
                             expansion_ptrs.end());
    }
  }
  assert(token_index == tokens.size());
  tokens.clear();
  return ErrorCode::Success();
}

}  // namespace pbc
