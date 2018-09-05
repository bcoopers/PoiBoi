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

#include <iostream>

#include "scanner.h"

namespace pbc {
namespace {

// A light string pointing into another string.
struct LightString {
  const char* begin = nullptr;
  const char* end = nullptr;
  size_t line_num = 0;

  LightString(const std::string& str, size_t start_idx, size_t end_idx,
              size_t line)
    : begin(&str[0] + start_idx),
      end(&str[0] + end_idx),
      line_num(line) {}

  LightString(const char* begin_c, const char* end_c, size_t line)
    : begin(begin_c),
      end(end_c),
      line_num(line) {}

  LightString() {}
};

// Either contains a QuotedString token, or a single line of code.
class LineOrQuotedString {
 public:
  explicit LineOrQuotedString(const LightString& line) : line_(line) {}

  explicit LineOrQuotedString(QuotedString str)
    : str_(std::move(str)),
      has_quoted_string_(true) {}

  bool IsQuotedString() const { return has_quoted_string_; }
  QuotedString& GetQuotedString() { return str_; }
  const QuotedString& GetQuotedString() const { return str_; }

  bool IsLine() const { return !has_quoted_string_; }
  const LightString& GetLine() const { return line_; }

 private:
  LightString line_;
  QuotedString str_;
  bool has_quoted_string_ = false;
};

// Advances the line and char iterator to the end of a comment. Comments are
// constained between two #s.
ErrorCode ProcessComment(const std::string& code, size_t& curr_line_num,
                         std::string::const_iterator& curr_char) {
  assert(*curr_char == '#');
  const size_t starting_line_num = curr_line_num;
  ++curr_char;
  for (; curr_char != code.end(); ++curr_char) {
    if (*curr_char == '\n') {
      ++curr_line_num;
    } else if (*curr_char == '#') {
      ++curr_char;
      return ErrorCode::Success();
    }
  }
  return ErrorCode::Failure("Line " + std::to_string(starting_line_num + 1) +
                            ": Comment starting here did not terminate.");
}

ErrorCode ProcessString(const std::string& code, size_t& curr_line_num,
                        std::string::const_iterator& curr_char,
                        QuotedString& qstr) {
  qstr = QuotedString();
  const size_t starting_line_num = curr_line_num;
  qstr.set_line_number(curr_line_num);
  assert(qstr.Search(*curr_char));
  for (; curr_char != code.end(); ++curr_char) {
    if (*curr_char == '\n') {
      ++curr_line_num;
    }
    if (!qstr.Search(*curr_char) && qstr.IsFinalizable()) {
      return ErrorCode::Success();
    }
  }
  return qstr.IsFinalizable()
         ? ErrorCode::Success()
         : ErrorCode::Failure("Line " + std::to_string(starting_line_num + 1) +
                              ": String starting here did not terminate.");
}

// Step 1 is to find strings, strip comments, and divide into lines and pieces
// around comments and strings.
ErrorCode RemoveCommentsAndParseStrings(
    const std::string& code,
    std::vector<LineOrQuotedString>& lines_and_strings) {
  lines_and_strings.clear();
  if (code.empty()) {
    return ErrorCode::Success();
  }
  size_t curr_line_num = 0;
  auto curr_char = code.begin();
  auto curr_line_begin = curr_char;
  while (curr_char != code.end()) {
    if (*curr_char == '"' || *curr_char == '#') {  // Process string or comment.
      if (curr_char != curr_line_begin) {
        lines_and_strings.emplace_back(
            LightString(&*curr_line_begin, &*curr_char, curr_line_num));
      }
      if (*curr_char == '"') {  // Process a string
        QuotedString qstr;
        RETURN_EC_IF_FAILURE(ProcessString(code, curr_line_num, curr_char,
                                           qstr));
        lines_and_strings.emplace_back(std::move(qstr));
      } else {  // Process a comment.
        RETURN_EC_IF_FAILURE(ProcessComment(code, curr_line_num, curr_char));
      }
      curr_line_begin = curr_char;
    } else {  // Process any other kind of characters.
      if (*curr_char == '\n') {
        lines_and_strings.emplace_back(
          LightString(&*curr_line_begin, (&*curr_char) + 1, curr_line_num));
        ++curr_line_num;
        curr_line_begin = curr_char + 1;
      }
      ++curr_char;
    }
  }
  return ErrorCode::Success();
}

bool IsWhitespace(const char c) {
  return c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '\f' ||
         c == '\v';
}

void StripWhitespaceFromLines(std::vector<LineOrQuotedString>& lines) {
  std::vector<LineOrQuotedString> lines_out;
  for (auto& line_in : lines) {
    if (line_in.IsQuotedString()) {
      lines_out.push_back(std::move(line_in));
    } else {
      const auto& line = line_in.GetLine();
      const size_t line_num = line.line_num;
      const char* cit = line.begin;
      const char* line_start = cit;
      while (cit != line.end) {
        if (IsWhitespace(*cit)) {
          if (cit != line_start) {
            lines_out.emplace_back(LightString(line_start, cit, line_num));
          }
          line_start = cit + 1;
        }
        ++cit;
      }
      if (line_start != line.end) {
        lines_out.emplace_back(LightString(line_start, line.end, line_num));
      }
    }
  }
  lines = std::move(lines_out);
}

std::vector<std::unique_ptr<TokenPiece>> CreateAllTokens() {
  std::vector<std::unique_ptr<TokenPiece>> tokens;
  tokens.emplace_back(new OpenCodeBlock());
  tokens.emplace_back(new CloseCodeBlock());
  tokens.emplace_back(new EndStatement());
  tokens.emplace_back(new OpenFunctionCall());
  tokens.emplace_back(new CloseFunctionCall());
  tokens.emplace_back(new ArgumentListSeparator());
  tokens.emplace_back(new OpenConditionalBlock());
  tokens.emplace_back(new CloseConditionalBlock());
  tokens.emplace_back(new Assigner());
  tokens.emplace_back(new KeywordLocal());
  tokens.emplace_back(new KeywordGlobal());
  tokens.emplace_back(new KeywordWhile());
  tokens.emplace_back(new KeywordIf());
  tokens.emplace_back(new KeywordElse());
  tokens.emplace_back(new KeywordElif());
  tokens.emplace_back(new KeywordReturn());
  tokens.emplace_back(new KeywordBreak());
  tokens.emplace_back(new QuotedString());
  tokens.emplace_back(new Variable());
  tokens.emplace_back(new Builtin());
  tokens.emplace_back(new FunctionName());
  tokens.emplace_back(new EndOfFile());
  return tokens;
}

ErrorCode ParseSingleToken(const size_t line_num, const char* line_end,
                           const char*& curr_char,
                           std::unique_ptr<TokenPiece>& parsed_token) {
  parsed_token.reset(nullptr);
  std::vector<std::unique_ptr<TokenPiece>> all_tokens = CreateAllTokens();
  all_tokens.erase(all_tokens.end() - 1);  // Get rid of EndOfFile token.
  const char* char_it = curr_char;
  for (; char_it != line_end; ++char_it) {
    bool any_successful_search = false;
    auto token_it = all_tokens.begin();
    while(token_it != all_tokens.end()) {
      const bool successful_search = (*token_it)->Search(*char_it);
      any_successful_search |= successful_search;
      if (!successful_search && !(*token_it)->IsFinalizable()) {
        token_it = all_tokens.erase(token_it);
      } else {
        ++token_it;
      }
    }
    if (!any_successful_search) {
      break;
    }
  }
  all_tokens.erase(std::remove_if(all_tokens.begin(), all_tokens.end(),
                                  [](const std::unique_ptr<TokenPiece>& token) {
                                    return !token->IsFinalizable();
                                  }), all_tokens.end());
  if (all_tokens.empty()) {
    return ErrorCode::Failure(
        "Line " + std::to_string(line_num + 1) +
        ": Couldn't parse token from:\n" +
        std::string(curr_char, line_end - curr_char));
  }
  // Return the longest token.
  auto longest_token = std::max_element(
      all_tokens.begin(), all_tokens.end(),
      [](const std::unique_ptr<TokenPiece>& token1,
         const std::unique_ptr<TokenPiece>& token2) {
           return token1->GetLength() < token2->GetLength();
         });
  curr_char += (*longest_token)->GetLength();
  parsed_token = std::move(*longest_token);
  parsed_token->set_line_number(line_num);
  return ErrorCode::Success();
}

ErrorCode ParseLinesAndStringsToTokens(
    std::vector<LineOrQuotedString>& lines_and_strings,
    std::vector<std::unique_ptr<TokenPiece>>& tokens) {
  for (LineOrQuotedString& line_or_string : lines_and_strings) {
    if (line_or_string.IsQuotedString()) {
      tokens.emplace_back(
          new QuotedString(std::move(line_or_string.GetQuotedString())));
    } else {
      const LightString& line = line_or_string.GetLine();
      const char* curr_char = line.begin;
      while (curr_char != line.end) {
        std::unique_ptr<TokenPiece> parsed_token;
        RETURN_EC_IF_FAILURE(
            ParseSingleToken(line.line_num, line.end, curr_char, parsed_token));
        tokens.push_back(std::move(parsed_token));
      }
    }
  }
  tokens.emplace_back(new EndOfFile());
  lines_and_strings.clear();
  return ErrorCode::Success();
}

}  // namespace

ErrorCode ScanTokens(const std::string& code,
                     std::vector<std::unique_ptr<TokenPiece>>& tokens) {
  tokens.clear();
  std::vector<LineOrQuotedString> lines_and_strings;
  RETURN_EC_IF_FAILURE(RemoveCommentsAndParseStrings(code, lines_and_strings));
  StripWhitespaceFromLines(lines_and_strings);
  return ParseLinesAndStringsToTokens(lines_and_strings, tokens);
}

}  // namespace pbc
