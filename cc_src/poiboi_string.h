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

#ifndef POIBOI_STRING_H_
#define POIBOI_STRING_H_

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <limits>
#include <utility>

// This represents all the possible string types (defined below).
enum TypeOfString {
  STATIC_STRING = 0,
  REF_COUNTED_STRING = 1,
  SMALL_STRING = 2,
  JOIN_RESULT = 3,
};

// String is allocated statically. No cleanup needed.
struct StaticString {
  const char* string;
  size_t length;
};

// String is reference counted.
struct RefCountedString {
  // This field serves two purposes:
  // 1) When dereferenced, it gives the number of references held.
  // 2) When the number of references held drops to 0, this is the address
  //    to delete.
  size_t* num_references_held;
  const char* string;
  size_t length;
};

// The largest possible small string is sizeof(RefCountedString) plus an extra
// byte to store length, or the maximum value of that extra byte.
inline constexpr size_t SmallStringMaxLength() {
  return std::numeric_limits<unsigned char>::max() >
         sizeof(struct RefCountedString) - 1
         ? sizeof(struct RefCountedString) - 1
         : std::numeric_limits<unsigned char>::max();
}

// String is small enough to fit in the payload.
struct SmallString {
  char string[SmallStringMaxLength()];
  unsigned char length;
};


// The payloads that can exist in a JoinResult. Everything except the JoinResult
// itself.
enum JoinType {
  JOINED_STATIC_STRING = 0,
  JOINED_REF_COUNTED_STRING = 1,
  JOINED_SMALL_STRING = 2,
};

union JoinPayload {
  struct StaticString static_string;
  struct RefCountedString ref_counted_string;
  struct SmallString small_string;
};


// The concatination of two of the above. Left is first, right is second.
struct JoinResult {
  union JoinPayload left_payload;
  union JoinPayload right_payload;
  enum JoinType left_type;
  enum JoinType right_type;
};

// A string is exactly one of the four types above.
union StringPayload {
  struct StaticString static_string;
  struct RefCountedString ref_counted_string;
  struct JoinResult join_result;
  struct SmallString small_string;
};

class PBString {
 public:
  // Constructors.
  PBString();
  PBString(const StaticString& s);

  // Static Initializers.
  // TODO: What if raw_string has '\0'?
  static PBString NewStaticString(const char* raw_string);
  static PBString True();
  static PBString False();
  static PBString Substring(const PBString& string, size_t start_index,
                            size_t end_index);
  static PBString Concat(const PBString& s1, const PBString& s2);
  static PBString SizeToString(size_t size);

  // Rule of 5- destructor, copy constructor, move constructor, copy assignment,
  // move assignment.
  // All related to managing ref counted strings.
  virtual ~PBString();
  PBString(const PBString& other);
  PBString(PBString&& other);
  PBString& operator=(const PBString& other);
  PBString& operator=(PBString&& other);

  size_t Length() const;

  // Access to raw string(s). rs2 will be nonnull if and only if type() is
  // equal to JOIN_RESULT.
  void GetRawStrings(char const *& rs1, char const*& rs2) const;

  // Access to lengths. This gives the lengths of rs1 and rs2 above.
  void GetLengths(size_t& l1, size_t& l2) const;

  // Two strings are equal if their raw strings are equal.
  bool operator==(const PBString& other) const;

  // Whether or not *this == TrueString().
  operator bool() const;

  TypeOfString type() const { return type_; }

  // Attempts to interpret this as a size. If it fails, returns false.
  bool StringToSize(size_t& out) const;

 private:
  // The following can crash if type_ is not correct.
  const char* RawStr() const;
  const char* LeftStr() const;
  const char* RightStr() const;
  size_t LeftLen() const;
  size_t RightLen() const;
  StringPayload payload_;
  TypeOfString type_;
};

PBString Builtin_Equal(const PBString& s1, const PBString& s2);

PBString Builtin_Print(const PBString& s);

inline PBString Builtin_Concat(const PBString& s1, const PBString& s2) {
  return PBString::Concat(s1, s2);
}

inline PBString Builtin_Not(const PBString& s) {
  PBString string_true = PBString::True();
  return s == string_true ? PBString::False() : string_true;
}

inline PBString Builtin_And(const PBString& s1, const PBString& s2) {
  PBString string_true = PBString::True();
  return s1 == string_true && s2 == string_true
         ? string_true : PBString::False();
}

inline PBString Builtin_Or(const PBString& s1, const PBString& s2) {
  PBString string_true = PBString::True();
  return s1 == string_true || s2 == string_true
         ? string_true : PBString::False();
}

inline PBString Builtin_Strlen(const PBString& s) {
  return PBString::SizeToString(s.Length());
}

PBString Builtin_Substring(
    const PBString& s, const PBString& start_str, const PBString& end_str);

#endif  // #ifndef POIBOI_STRING_H_
