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

#include "tokens.h"

namespace pbc {

bool QuotedString::Search(char c) {
  if (no_more_) {
    return false;
  }
  if (c == '\n') {
    no_more_ = true;
    return false;
  }
  if (content_.empty()) {
    if (c == '"') {
      content_.push_back(c);
      return true;
    }
    no_more_ = true;
    return false;
  }
  if (c == '"' && num_backslashes_in_row_ % 2 == 0) {
    no_more_ = true;
    is_finalized_ = true;
  } else if (c == '\\') {
    ++num_backslashes_in_row_;
  } else {
    num_backslashes_in_row_ = 0;
  }
  content_.push_back(c);
  return true;
}

bool Variable::Search(char c) {
  if (no_more_) {
    return false;
  }
  if ((c >= 'a' && c <= 'z') ||
      (!content_.empty() && c >= 'A' && c <= 'Z')) {
    content_.push_back(c);
    return true;
  }
  no_more_ = true;
  return false;
}

bool Builtin::Search(char c) {
  if (no_more_) {
    return false;
  }
  if (c >= 'A' && c <= 'Z') {
    content_.push_back(c);
    return true;
  }
  no_more_ = true;
  return false;
}

bool FunctionName::Search(char c) {
  if (no_more_) {
    return false;
  }
  if (c >= 'a' && c <= 'z' && !content_.empty()) {
    contains_lower_case_ = true;
    content_.push_back(c);
    return true;
  }
  if (c >= 'A' && c <= 'Z') {
    content_.push_back(c);
    return true;
  }
  no_more_ = true;
  return false;
}

}  // namespace pbc
