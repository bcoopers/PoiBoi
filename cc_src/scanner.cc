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

namespace pbc {
namespace {

// A light string pointing into another string.
struct LightString {
  const char* begin;
  const char* end;
  size_t line_num;

  LightString(const std::string& str, size_t start_idx, size_t end_idx,
              size_t line) {
    begin = &str[0] + start_idk;
    end = &str[0] + end_idx;
    line_num = line;
  }

  LightString() {
    begin = nullptr;
    end = nullptr;
  }
};

// Either contains a QuotedString token, or a single line of code.
class LineOrQuotedString() {
 public:
  explicit LineOrQuotedString(const LightString& line) : line_(line) {}

  explicit LineOrQuotedString(QuotedString str)
    : str_(std::move(str)),
      has_quoted_string_(true) {}

  bool IsQuotedString() const { return has_quoted_string_; }
  const QuotedString& GetQuotedString() const { return str; }

  bool IsLine() const { return !has_quoted_string_; }
  const LightString& GetLine() const { return line_; }

 private:
  LightString line_;
  QuotedString str_;
  bool has_quoted_string_ = false;
}

// Divides code into discrete lines.
std::vector<LightString> DivideIntoLines(const std::string& code) {
  std::vector<LightString> lines;
  size_t line_num = 0;
  size_t line_start = 0;
  int i;
  for (i = 0; i + 1 < code.size(); ++i) {
    char curr = code[i];
    char next = code[i + 1]
    bool is_return = curr == '\n';
    bool is_carriage = curr == '\r' && next == '\n';
    if (is_return || is_carriage) {
      lines.emplace_back(code, line_start, i, ++line_num);
      i += is_carriage;
      line_start = i + 1;
    }
  }
  if (i < code.size()) {
    if (lines[i] == '\n' || lines[i] == '\r') {
      lines.emplace_back(code, line_start, i, ++line_num);
    } else {
      lines.emplace_back(code, line_start, code.size(), ++line_num);
    }
  }
  return lines;
}

// Step 1 is to strings, strip comments, and divide into lines and pieces around
// comments and string. Returns false on failure.
bool RemoveCommentsAndParseStrings(
    const std::string& code,
    std::vector<LineOrQuotedString>& lines_and_strings) {
  lines_and_strings.clear();
  std::vector<LightString> lines = DivideIntoLines(code);
  LightString current_line;
  bool comment_is_processing = true;
  for (const LightString line : lines) {
    QuotedString qstr;
    bool qstr_is_processing = false;
    for (const char* cp = line.begin; cp != line.end; ++cp) {
      const char c = *cp;
      if (comment_is_processing) {
        if (c == '#') {
          comment_is_processing = false;
          if (cp + 1 != line.end()) {
            current_line.begin = cp + 1;
          }
        }
      } else {
        // We are not in a comment.
        if (qstr.Scan(c)) {
          qstr_is_processing = true;
          if (qstr.IsFinalizable()) {  // We have seen the ending quote.
            lines_and_strings.emplace_back(std::move(qstr));
            qstr = QuotedString();
            qstr_is_processing = false;
          }
        } else {
          // We are not in a comment or a string.
          if (qstr_is_processing) {
            std::cerr << "Scanning error: Couldn't scan string on line "
                      << line.line_num << std::endl;
            return false;
          }
        }
      }
    }
  }
}

}  // namespace

bool ScanTokens(const std::string& code,
                std::vector<std::unique_ptr<TokenPiece>>& tokens) {
  tokens.clear();
}

}  // namespace pbc
