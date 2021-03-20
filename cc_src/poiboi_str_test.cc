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
#include <cassert>
#include <random>
#include <string>

#include "poiboi_string.h"

namespace {
void ConcatTest() {
  const PBString hello = PBString::NewStaticString("Hello ");
  const PBString world = PBString::NewStaticString("World!");
  const PBString hello_world1 = PBString::NewStaticString("Hello World!");
  const PBString hello_world2 = Builtin_Concat(hello, world);
  assert(hello_world1 == hello_world2);

  const PBString double_concat1 = Builtin_Concat(hello_world1, hello_world2);
  const PBString double_concat2 = Builtin_Concat(hello_world1, hello_world1);
  const PBString double_concat3 = Builtin_Concat(hello_world2, hello_world2);
  const PBString double_concat4 = Builtin_Concat(hello_world2, hello_world1);
  PBString expected = PBString::NewStaticString("Hello World!Hello World!");
  assert(double_concat1 == expected);
  assert(double_concat2 == expected);
  assert(double_concat3 == expected);
  assert(double_concat4 == expected);
  expected = PBString::NewStaticString("Hello World!Hello World!Hello World!");
  assert(Builtin_Concat(double_concat3, hello_world1) == expected);
  assert(Builtin_Concat(double_concat3, hello_world2) == expected);
  assert(Builtin_Concat(hello_world1, double_concat3) == expected);
  assert(Builtin_Concat(hello_world2, double_concat3) == expected);
}

void HugeConcatTest() {
  constexpr size_t kPowerOfTwo = 262144;
  char expected[kPowerOfTwo + 1];
  for (size_t i = 0; i < kPowerOfTwo; i += 8) {
    expected[i] = 'H';
    expected[i + 1] = 'e';
    expected[i + 2] = 'l';
    expected[i + 3] = 'l';
    expected[i + 4] = 'o';
    expected[i + 5] = ' ';
    expected[i + 6] = 'Y';
    expected[i + 7] = '0';
  }
  expected[kPowerOfTwo] = 0;
  PBString expected_s = PBString::NewStaticString(expected);
  PBString hello_yo = PBString::NewStaticString("Hello Y0");
  PBString concat_result = hello_yo;
  constexpr size_t kLog = 15;
  for (size_t i = 0; i < kLog; ++i) {
    concat_result = Builtin_Concat(concat_result, concat_result);
  }
  assert(concat_result.Length() == expected_s.Length());
  assert(concat_result == expected_s);
  PBString string_len = Builtin_Strlen(concat_result);
  PBString expected_strlen = PBString::SizeToString(kPowerOfTwo);
  assert(string_len == expected_strlen);
}

void LogicTest() {
  const PBString true_str = PBString::True();
  const PBString false_str = PBString::False();

  assert(Builtin_Equal(true_str, true_str) == true_str);
  assert(Builtin_Equal(true_str, false_str) == false_str);
  assert(Builtin_Equal(false_str, true_str) == false_str);
  assert(Builtin_Equal(false_str, false_str) == true_str);

  assert(Builtin_Not(false_str) == true_str);
  assert(Builtin_Not(true_str) == false_str);

  assert(Builtin_And(true_str, true_str) == true_str);
  assert(Builtin_And(true_str, false_str) == false_str);
  assert(Builtin_And(false_str, true_str) == false_str);
  assert(Builtin_And(false_str, false_str) == false_str);

  assert(Builtin_Or(true_str, true_str) == true_str);
  assert(Builtin_Or(true_str, false_str) == true_str);
  assert(Builtin_Or(false_str, true_str) == true_str);
  assert(Builtin_Or(false_str, false_str) == false_str);
}

void StrLenTest() {
  constexpr size_t kMaxTest = 100010;
  char str[kMaxTest];
  for (size_t i = 0; i < kMaxTest; ++i) {
    str[i] = 0;
    const PBString x = PBString::NewStaticString(str);
    const PBString slen = Builtin_Strlen(x);
    size_t len;
    assert(slen.StringToSize(len));
    assert(len == i);
    str[i] = 'x';
  }
}

void SubstringIndicesTest() {
  const PBString s1 = PBString::NewStaticString("abcdefgh");
  assert(s1 == Builtin_Substring(s1, s1, s1));
  PBString expected = PBString::NewStaticString("cdefg");
  PBString i1 = PBString::NewStaticString("2");
  PBString i2 = PBString::NewStaticString("7");
  assert(expected == Builtin_Substring(s1, i1, i2));
  i1 = PBString::NewStaticString("7");
  i2 = PBString::NewStaticString("2");
  expected = PBString();
  assert(expected == Builtin_Substring(s1, i1, i2));
  i1 = PBString::NewStaticString("4");
  i2 = PBString::NewStaticString("100");
  expected = PBString::NewStaticString("efgh");
  assert(expected == Builtin_Substring(s1, i1, i2));
  i1 = PBString::NewStaticString("100");
  i2 = PBString::NewStaticString("8");
  expected = PBString();
  assert(expected == Builtin_Substring(s1, i1, i2));
}

void HugeSubstringTest() {
  const PBString s1 = PBString::NewStaticString("abcdefgh");
  const std::string std1("abcdefgh");
  PBString concat = s1;
  std::string stdconcat = std1;
  for (int i = 0; i < 1000; ++i) {
    concat = Builtin_Concat(concat, concat);
    concat = Builtin_Concat(concat, concat);
    concat = Builtin_Concat(concat, concat);
    stdconcat = stdconcat + stdconcat;
    stdconcat = stdconcat + stdconcat;
    stdconcat = stdconcat + stdconcat;
    PBString expected = PBString::NewStaticString(stdconcat.c_str());
    assert(concat == expected);
    size_t index2 = rand() % concat.Length();
    size_t index1 = index2 == 0 ? 0 : rand() % index2;
    while (index2 - index1 < 1 || index2 - index1 >= 2500000) {
      index2 = rand() % concat.Length();
      index1 = index2 == 0 ? 0 : rand() % index2;
    }
    PBString index2_str = PBString::SizeToString(index2);
    PBString index1_str = PBString::SizeToString(index1);
    concat = Builtin_Substring(concat, index1_str, index2_str);
    stdconcat = std::string(stdconcat.begin() + index1,
                            stdconcat.begin() + index2);
    expected = PBString::NewStaticString(stdconcat.c_str());
    assert(expected == concat);
  }
}
}  // namespace

int main() {
  srand(100);
  ConcatTest();
  HugeConcatTest();
  LogicTest();
  StrLenTest();
  SubstringIndicesTest();
  HugeSubstringTest();
  return 0;
}
