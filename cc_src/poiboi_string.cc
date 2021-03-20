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

#ifndef POIBOI_EXECUTABLE_
// Don't need to include the .h file in a PoiBoi executable, everything will
// be concatted into one file.
#include "poiboi_string.h"
#else
// Always want to use assertions in a non-executable. In the executable it may
// be triggered by compilation mode.
#define POIBOI_INCLUDE_ASSERT_
#endif

#ifdef POIBOI_INCLUDE_ASSERT_
#define ASSERT(x) assert(x)
#define CRASH_RETURN(x) assert(false); return x
#else
#define ASSERT(X)
#define CRASH_RETURN(x) return x
#endif  // #ifdef INCLUDE_ASSERT

namespace {
// TODO: The three functions below should be made thread safe.

// Returns a mutable char* that can be written to. s should not own any
// references to any strings before calling this function.

char* NewRefCountedString(size_t num_chars, RefCountedString& s) {
  ASSERT(num_chars > 0);
  char* memory = (char*)malloc(num_chars + sizeof(size_t));
  s.num_references_held = (size_t*) memory;
  (*s.num_references_held) = 1;
  s.string = memory + sizeof(size_t);
  s.length = 0;
  return memory + sizeof(size_t);
}

// Decrements the reference counter. If it is 0, the string is freed.
void CleanupRefCountedString(RefCountedString& str) {
  ASSERT(*str.num_references_held > 0);
  --(*str.num_references_held);
  if (*str.num_references_held == 0) {
    free((void*)str.num_references_held);
  }
}

// Copies in to out and increments the reference counter.
RefCountedString CopyRefCountedString(const RefCountedString& in) {
  RefCountedString out;
  ASSERT(*in.num_references_held > 0);
  out = in;
  ++(*out.num_references_held);
  return out;
}

// Copies a JoinPayload. Also
JoinPayload CopyJoinPayload(
    const JoinType in_type, const JoinPayload& in_jp) {
  JoinPayload out_jp;
  switch (in_type) {
    case JOINED_REF_COUNTED_STRING:
      out_jp.ref_counted_string =
          CopyRefCountedString(in_jp.ref_counted_string);
      break;
    case JOINED_STATIC_STRING: case JOINED_SMALL_STRING:
      out_jp = in_jp;
      break;
  }
  return out_jp;
}

JoinResult CopyJoinResult(const JoinResult& in) {
  JoinResult out;
  out.left_type = in.left_type;
  out.right_type = in.right_type;
  out.left_payload = CopyJoinPayload(in.left_type, in.left_payload);
  out.right_payload = CopyJoinPayload(in.right_type, in.right_payload);
  return out;
}

StringPayload CopyStringPayload(const TypeOfString in_type,
                                const StringPayload& in_sp) {
  StringPayload out_sp;
  switch (in_type) {
    case REF_COUNTED_STRING:
      out_sp.ref_counted_string =
          CopyRefCountedString(in_sp.ref_counted_string);
      break;
    case JOIN_RESULT:
      out_sp.join_result = CopyJoinResult(in_sp.join_result);
      break;
    case SMALL_STRING: case STATIC_STRING:
      out_sp = in_sp;
      break;
  }
  return out_sp;
}

StringPayload JoinPayloadToStringPayload(
  const JoinType in_type, const JoinPayload& in_jp) {
  StringPayload out_sp;
  switch (in_type) {
    case JOINED_STATIC_STRING:
      out_sp.static_string = in_jp.static_string;
      break;
    case JOINED_SMALL_STRING:
      out_sp.small_string = in_jp.small_string;
      break;
    case JOINED_REF_COUNTED_STRING:
      out_sp.ref_counted_string =
          CopyRefCountedString(in_jp.ref_counted_string);
      break;
  }
  return out_sp;
}

JoinPayload StringPayloadToJoinPayload(const StringPayload& sp,
                                       TypeOfString type) {
  JoinPayload jp;
  switch(type) {
    case STATIC_STRING:
      jp.static_string = sp.static_string;
      break;
    case SMALL_STRING:
      jp.small_string = sp.small_string;
      break;
    case REF_COUNTED_STRING:
      jp.ref_counted_string = CopyRefCountedString(sp.ref_counted_string);
      break;
    case JOIN_RESULT:
      CRASH_RETURN(jp);
  }
  return jp;
}

void CleanupStringPayload(TypeOfString type, StringPayload& sp) {
  if (type == REF_COUNTED_STRING) {
    CleanupRefCountedString(sp.ref_counted_string);
  } else if (type == JOIN_RESULT) {
    // Cleanup any possible ref counted strings in the join result.
    JoinResult& jr = sp.join_result;
    if (jr.left_type == JOINED_REF_COUNTED_STRING) {
      CleanupRefCountedString(jr.left_payload.ref_counted_string);
    }
    if (jr.right_type == JOINED_REF_COUNTED_STRING) {
      CleanupRefCountedString(jr.right_payload.ref_counted_string);
    }
  }
}


// Will crash if the input is JOIN_RESULT, which is not a valid join payload.
JoinType TypeOfStringToJoinType(const TypeOfString t) {
  switch (t) {
    case STATIC_STRING:
      return JOINED_STATIC_STRING;
    case REF_COUNTED_STRING:
      return JOINED_REF_COUNTED_STRING;
    case SMALL_STRING:
      return JOINED_SMALL_STRING;
    case JOIN_RESULT:
      CRASH_RETURN(JOINED_STATIC_STRING);
  }
}

TypeOfString JoinTypeToTypeOfString(const JoinType t) {
  switch (t) {
    case JOINED_STATIC_STRING:
      return STATIC_STRING;
    case JOINED_REF_COUNTED_STRING:
      return REF_COUNTED_STRING;
    case JOINED_SMALL_STRING:
      return SMALL_STRING;
  }
}

// Returns the length of a join payload.
size_t JoinPayloadLength(JoinType type, const JoinPayload& jp) {
  switch (type) {
    case JOINED_STATIC_STRING:
      return jp.static_string.length;
    case JOINED_REF_COUNTED_STRING:
      return jp.ref_counted_string.length;
    case JOINED_SMALL_STRING:
      return jp.small_string.length;
  }
}
size_t LeftLength(const JoinResult& jp) {
  return JoinPayloadLength(jp.left_type, jp.left_payload);
}
size_t RightLength(const JoinResult& jp) {
  return JoinPayloadLength(jp.right_type, jp.right_payload);
}
size_t JoinLength(const JoinResult& jp) {
  return LeftLength(jp) + RightLength(jp);
}
const char* JoinPayloadRawString(JoinType type, const JoinPayload& jp) {
  switch (type) {
    case JOINED_STATIC_STRING:
      return jp.static_string.string;
    case JOINED_REF_COUNTED_STRING:
      return jp.ref_counted_string.string;
    case JOINED_SMALL_STRING:
      return jp.small_string.string;
  }
}
const char* LeftRawString(const JoinResult& jp) {
  return JoinPayloadRawString(jp.left_type, jp.left_payload);
}
const char* RightRawString(const JoinResult& jp) {
  return JoinPayloadRawString(jp.right_type, jp.right_payload);
}

// Returns whether short_left equals long_left. Requires that both arguments
// have the same length, and the first argument's left string is shorter
// than the second argument's left string
bool ShortLongJoinResultsEqual(const JoinResult& short_left,
                               const JoinResult& long_left) {
  ASSERT(LeftLength(short_left) < LeftLength(long_left));
  ASSERT(JoinLength(short_left) == JoinLength(long_left));
  return memcmp(LeftRawString(short_left), LeftRawString(long_left),
                LeftLength(short_left)) != 0 &&
         memcmp(LeftRawString(long_left) + LeftLength(short_left),
                RightRawString(short_left),
                LeftLength(long_left) - LeftLength(short_left)) != 0 &&
         memcmp(RightRawString(short_left) + LeftLength(long_left)
                - LeftLength(short_left),
                RightRawString(long_left), RightLength(long_left)) == 0;
}

// Returns true if two join results contain the same string.
bool JoinResultsEqual(const JoinResult& s1, const JoinResult& s2) {
  if (JoinLength(s1) != JoinLength(s2)) {
    return false;
  }

  if (LeftLength(s1) == LeftLength(s2)) {
    return memcmp(LeftRawString(s1), LeftRawString(s2), LeftLength(s1)) == 0 &&
           memcmp(RightRawString(s1), RightRawString(s2), RightLength(s1)) == 0;
  }
  if (LeftLength(s1) < LeftLength(s2)) {
    return ShortLongJoinResultsEqual(s1, s2);
  }
  return ShortLongJoinResultsEqual(s2, s1);
}

// Returns true if the join_result contains the same characters as the raw
// string.
bool JoinResultEqualsRaw(const JoinResult& join_result,
                         const char* string, const size_t length) {
  size_t left_length = LeftLength(join_result);
  size_t right_length = RightLength(join_result);
  if (length != left_length + right_length) {
    return false;
  }
  const char* left_str = LeftRawString(join_result);
  const char* right_str = RightRawString(join_result);
  return memcmp(string, left_str, left_length) == 0 &&
         memcmp(string + left_length, right_str, right_length) == 0;
}

// Shifts the payload shift_amount characters to the right. Note that the
// input type is JoinType, so this function won't work with payloads of type
// JoinResult.
template<typename Payload>
void PayloadShiftRight(const size_t shift_amount, const JoinType type,
                       Payload& payload) {
  switch (type) {
    case JOINED_STATIC_STRING:
      payload.static_string.string += shift_amount;
      payload.static_string.length -= shift_amount;
      break;
    case JOINED_REF_COUNTED_STRING:
      payload.ref_counted_string.string += shift_amount;
      payload.ref_counted_string.length -= shift_amount;
      break;
    case JOINED_SMALL_STRING:
      auto& length = payload.small_string.length;
      memmove(payload.small_string.string,
              payload.small_string.string + shift_amount,
              length - shift_amount);
      length -= shift_amount;
      break;
  }
}

// Decrements the payload length by dec. Note that the input type is JoinType,
// so this function won't work with payloads of type JoinResult.
template<typename Payload>
void DecrementPayloadLength(size_t dec, JoinType type, Payload& jp) {
  switch (type) {
    case JOINED_STATIC_STRING:
      ASSERT(jp.static_string.length >= dec);
      jp.static_string.length -= dec;
      break;
    case JOINED_REF_COUNTED_STRING:
      ASSERT(jp.ref_counted_string.length >= dec);
      jp.ref_counted_string.length -= dec;
      break;
    case JOINED_SMALL_STRING:
      ASSERT(jp.small_string.length >= dec);
      jp.small_string.length -= dec;
      break;
  }
}

// Returns the substring of a JoinResult. This can be any type at all, which
// is also outputted into out_type.
StringPayload SubstringOfJoinResult(
    const JoinResult& in, size_t start_index, size_t end_index,
    TypeOfString& out_type) {
  StringPayload out_payload;
  const size_t in_left_length = LeftLength(in);
  const size_t in_right_length = RightLength(in);
  const size_t in_length = in_left_length + in_right_length;
  ASSERT(start_index < end_index);
  ASSERT(end_index <= in_left_length + in_right_length);
  const size_t new_length = end_index - start_index;
  if (end_index <= in_left_length || start_index >= in_left_length) {
    // The substring is fully in left or right.
    if (end_index <= in_left_length) {
      // Substring is fully in left.
      out_type = JoinTypeToTypeOfString(in.left_type);
      out_payload = JoinPayloadToStringPayload(in.left_type, in.left_payload);
      PayloadShiftRight(start_index, in.left_type, out_payload);
      DecrementPayloadLength(in_left_length - end_index, in.left_type,
                             out_payload);
      return out_payload;
    }
    // Substring is fully in right.
    out_type = JoinTypeToTypeOfString(in.right_type);
    out_payload = JoinPayloadToStringPayload(in.right_type, in.right_payload);
    PayloadShiftRight(start_index - in_left_length, in.right_type, out_payload);
    DecrementPayloadLength(in_length - end_index, in.right_type, out_payload);
    return out_payload;
  }
  // The substring is in both the left and right.
  if (new_length <= SmallStringMaxLength()) {
    // Output a small string.
    out_type = SMALL_STRING;
    size_t first_bytes = in_left_length - start_index;
    memcpy(out_payload.small_string.string, LeftRawString(in) + start_index,
           first_bytes);
    memcpy(out_payload.small_string.string + first_bytes,
           RightRawString(in), new_length - first_bytes);
           out_payload.small_string.length = new_length;
    return out_payload;
  }
  // Output a new JoinResult.
  out_type = JOIN_RESULT;
  out_payload.join_result = CopyJoinResult(in);
  JoinResult& out_join = out_payload.join_result;
  PayloadShiftRight(start_index, in.left_type, out_join.left_payload);
  DecrementPayloadLength(in_length - end_index, in.right_type,
                         out_join.right_payload);
  return out_payload;
}

const char* AllThreeDigitNumbers() {
  char* nums = (char*)malloc(3 * 900 + 1);
  for (int i = 100; i < 1000; ++i) {
    int index = (i - 100) * 3;
    nums[index + 2] = i % 10 + '0';
    nums[index + 1] = (i / 10) % 10 + '0';
    nums[index] = (i / 100) % 10 + '0';
  }
  return nums;
}

const char* SmallSizeToRawString(size_t size) {
  static const char* all_three_digit_nums = AllThreeDigitNumbers();
  ASSERT(size < 1000);
  if (size < 10) {
    return all_three_digit_nums + 3 * size + 2;
  } else if (size < 100) {
    return all_three_digit_nums + 3 * size + 1;
  }
  return  all_three_digit_nums + 3 * (size - 100);
}

// If we have more chars then this, we don't check if it parses to a number.
constexpr size_t MaxSizeNumChars() {
  static_assert(sizeof(size_t) == 8 || sizeof(size_t) == 4 ||
                sizeof(size_t) == 2 || sizeof(size_t) == 1,
                "Expected bytes to be a power of 2 in [1, 8].");
  return sizeof(size_t) == 8 ? 20 :
         sizeof(size_t) == 4 ? 10 :
         sizeof(size_t) == 2 ? 5 :
         sizeof(size_t) == 1 ? 3 :
         0;
}

void WriteToString(const char* input1, size_t length1, const char* input2,
                   size_t length2, const char* input3, size_t length3,
                   const char* input4, size_t length4, char* out) {
   memcpy(out, input1, length1);
   size_t bytes_written = length1;
   memcpy(out + bytes_written, input2, length2);
   bytes_written += length2;
   memcpy(out + bytes_written, input3, length3);
   bytes_written += length3;
   memcpy(out + bytes_written, input4, length4);
}
}  // namespace

PBString::PBString() {
  type_ = SMALL_STRING;
  payload_.small_string.length = 0;
}

PBString PBString::NewStaticString(const char* raw_string) {
  PBString s;
  s.type_ = STATIC_STRING;
  s.payload_.static_string.length = strlen(raw_string);
  s.payload_.static_string.string = raw_string;
  return s;
}
PBString PBString::True() {
  PBString s;
  s.type_ = STATIC_STRING;
  s.payload_.static_string.length = 4;
  s.payload_.static_string.string = "TRUE";
  return s;
}

PBString PBString::False() {
  PBString s;
  s.type_ = STATIC_STRING;
  s.payload_.static_string.length = 5;
  s.payload_.static_string.string = "FALSE";
  return s;
}

PBString PBString::Substring(const PBString& string, size_t start_index,
                             size_t end_index) {
  PBString substr;
  size_t string_length = string.Length();
  if (end_index > string_length) {
    end_index = string_length;
  }
  if (start_index >= end_index) {
    return substr;
  }
  switch(string.type_) {
    case STATIC_STRING:
      substr = string;
      PayloadShiftRight(start_index, JOINED_STATIC_STRING, substr.payload_);
      DecrementPayloadLength(string_length - end_index, JOINED_STATIC_STRING,
                             substr.payload_);
      return substr;
    case REF_COUNTED_STRING:
      substr = string;
      PayloadShiftRight(start_index, JOINED_REF_COUNTED_STRING, substr.payload_);
      DecrementPayloadLength(string_length - end_index,
                             JOINED_REF_COUNTED_STRING, substr.payload_);
      return substr;
    case SMALL_STRING:
      ASSERT(string_length <= SmallStringMaxLength());
      substr = string;
      PayloadShiftRight(start_index, JOINED_SMALL_STRING, substr.payload_);
      DecrementPayloadLength(string_length - end_index, JOINED_SMALL_STRING,
                             substr.payload_);
      return substr;
    case JOIN_RESULT:
      substr.payload_ = SubstringOfJoinResult(
        string.payload_.join_result, start_index, end_index, substr.type_);
      return substr;
   }
}

PBString PBString::Concat(const PBString& s1, const PBString& s2) {
  size_t length_s1 = s1.Length();
  size_t length_s2 = s2.Length();
  size_t result_length = length_s1 + length_s2;
  if (length_s1 == 0) {
    return s2;
  } else if (length_s2 == 0) {
    return s1;
  }
  PBString ret;
  if (s1.type_ == JOIN_RESULT || s2.type_ == JOIN_RESULT) {
    ret.type_ = REF_COUNTED_STRING;
    char* write_to = NewRefCountedString(
        result_length + 1, ret.payload_.ref_counted_string);
    ret.payload_.ref_counted_string.length = result_length;
    if (s1.type_ == JOIN_RESULT && s2.type_ == JOIN_RESULT) {
      WriteToString(s1.LeftStr(), s1.LeftLen(), s1.RightStr(),
                    s1.RightLen(), s2.LeftStr(), s2.LeftLen(),
                    s2.RightStr(), s2.RightLen(), write_to);
    } else if (s1.type_ == JOIN_RESULT) {
      WriteToString(s1.LeftStr(), s1.LeftLen(), s1.RightStr(),
                    s1.RightLen(), s2.RawStr(), s2.Length(), nullptr, 0,
                    write_to);
    } else {
      WriteToString(s1.RawStr(), s1.Length(), s2.LeftStr(),
      s2.LeftLen(), s2.RightStr(), s2.RightLen(), nullptr, 0,
      write_to);
    }
    return ret;
  }
  if (result_length <= SmallStringMaxLength()) {
    ret.type_ = SMALL_STRING;
    ret.payload_.small_string.length = result_length;
    memcpy(ret.payload_.small_string.string, s1.RawStr(), length_s1);
    memcpy(ret.payload_.small_string.string + length_s1, s2.RawStr(),
           length_s2);
    return ret;
  }
  ret.type_ = JOIN_RESULT;
  JoinResult& jr = ret.payload_.join_result;
  jr.left_type = TypeOfStringToJoinType(s1.type_);
  jr.right_type = TypeOfStringToJoinType(s2.type_);
  jr.left_payload = StringPayloadToJoinPayload(s1.payload_, s1.type_);
  jr.right_payload = StringPayloadToJoinPayload(s2.payload_, s2.type_);
  return ret;
}

PBString PBString::SizeToString(size_t size) {
  PBString size_str;
  if (size < 1000) {
    size_str.type_ = STATIC_STRING;
    size_str.payload_.static_string.string = SmallSizeToRawString(size);
    if (size < 10) {
      size_str.payload_.static_string.length = 1;
    } else if (size < 100) {
      size_str.payload_.static_string.length = 2;
    } else {
      size_str.payload_.static_string.length = 3;
    }
    return size_str;
  }
  char* write_to;

  constexpr int kBufferSize = MaxSizeNumChars() + 1;

  if (kBufferSize > SmallStringMaxLength()) {
    size_str.type_ = REF_COUNTED_STRING;
    write_to = NewRefCountedString(
        kBufferSize, size_str.payload_.ref_counted_string);
  } else {
    size_str.type_ = SMALL_STRING;
    write_to = size_str.payload_.small_string.string;
  }
  int result = snprintf(write_to, kBufferSize, "%zu", size);
  if (result < kBufferSize && result >= 0) {
    if (size_str.type() == REF_COUNTED_STRING) {
      size_str.payload_.ref_counted_string.length = result;
    } else {
      ASSERT(size_str.type() == SMALL_STRING);
      size_str.payload_.small_string.length = result;
    }
  } else {
    CRASH_RETURN(size_str);
  }
  return size_str;
}

PBString::~PBString() {
  CleanupStringPayload(type_, payload_);
}

PBString::PBString(const PBString& other) {
  type_ = other.type_;
  payload_ = CopyStringPayload(other.type_, other.payload_);
}

PBString::PBString(PBString&& other) {
  type_ = other.type_;
  payload_ = other.payload_;

  other.type_ = SMALL_STRING;
  other.payload_.small_string.length = 0;
}

PBString& PBString::operator=(const PBString& other) {
  if (this == &other) {
    return *this;
  }
  CleanupStringPayload(type_, payload_);

  type_ = other.type_;
  payload_ = CopyStringPayload(other.type_, other.payload_);

  return *this;
}

PBString& PBString::operator=(PBString&& other) {
  if (this == &other) {
    return *this;
  }
  CleanupStringPayload(type_, payload_);

  type_ = other.type_;
  payload_ = other.payload_;

  other.type_ = SMALL_STRING;
  other.payload_.small_string.length = 0;

  return *this;
}

size_t PBString::Length() const {
  switch (type_) {
    case STATIC_STRING:
      return payload_.static_string.length;
    case REF_COUNTED_STRING:
      return payload_.ref_counted_string.length;
    case SMALL_STRING:
      return payload_.small_string.length;
    case JOIN_RESULT:
      return JoinLength(payload_.join_result);
  }
}

const char* PBString::RawStr() const {
    switch (type_) {
      case STATIC_STRING:
        return payload_.static_string.string;
      case REF_COUNTED_STRING:
        return payload_.ref_counted_string.string;
      case SMALL_STRING:
        return payload_.small_string.string;
      case JOIN_RESULT:
        CRASH_RETURN(nullptr);
    }
}

void PBString::GetRawStrings(const char*& rs1, const char*& rs2) const {
  switch (type_) {
    case STATIC_STRING: case REF_COUNTED_STRING: case SMALL_STRING:
      rs1 = RawStr();
      rs2 = nullptr;
      break;
    case JOIN_RESULT:
      rs1 = LeftStr();
      rs2 = RightStr();
      break;
  }
}

void PBString::GetLengths(size_t& l1, size_t& l2) const {
  switch (type_) {
    case STATIC_STRING: case REF_COUNTED_STRING: case SMALL_STRING:
      l1 = Length();
      l2 = 0;
      break;
    case JOIN_RESULT:
      l1 = LeftLen();
      l2 = RightLen();
      break;
  }
}

const char* PBString::LeftStr() const {
  ASSERT(type_ == JOIN_RESULT);
  return LeftRawString(payload_.join_result);
}

const char* PBString::RightStr() const {
  ASSERT(type_ == JOIN_RESULT);
  return RightRawString(payload_.join_result);
}

size_t PBString::LeftLen() const {
  ASSERT(type_ == JOIN_RESULT);
  return LeftLength(payload_.join_result);
}

size_t PBString::RightLen() const {
  ASSERT(type_ == JOIN_RESULT);
  return RightLength(payload_.join_result);
}

bool PBString::operator==(const PBString& other) const {
  const size_t length = Length();
  if (length != other.Length()) {
    return false;
  }
  if (type_ != JOIN_RESULT && other.type_ != JOIN_RESULT) {
    return memcmp(RawStr(), other.RawStr(), length) == 0;
  }
  if (other.type_ == JOIN_RESULT && this->type_ == JOIN_RESULT) {
    return JoinResultsEqual(payload_.join_result, other.payload_.join_result);
  }
  if (other.type_ == JOIN_RESULT) {
    return JoinResultEqualsRaw(other.payload_.join_result, RawStr(),
                               length);
  }
  return JoinResultEqualsRaw(payload_.join_result, other.RawStr(),
                             other.Length());
}

PBString::operator bool() const {
  return *this == PBString::True();
}

bool PBString::StringToSize(size_t& out) const {
  out = 0;
  size_t string_length = Length();
  if (string_length >= MaxSizeNumChars() || string_length == 0) {
    return false;
  }
  constexpr int kBufferSize = MaxSizeNumChars() + 1;
  char join_buffer[kBufferSize];
  const char* raw_string;
  if (type_ != JOIN_RESULT) {
    // TODO: We should be able to avoid this copy.
    memcpy(join_buffer, RawStr(), string_length);
    join_buffer[string_length] = 0;
    raw_string = join_buffer;
  } else {
    size_t left_length = LeftLen();
    memcpy(join_buffer, LeftStr(), left_length);
    memcpy(join_buffer + left_length, RightStr(), RightLen());
    join_buffer[string_length] = 0;
    raw_string = join_buffer;
  }
  for (size_t i = 0; i < string_length; ++i) {
    if (raw_string[i] < '0' || raw_string[i] > '9') {
      return false;
    }
  }
  return sscanf(raw_string, "%zu", &out) == 1;
}

PBString Builtin_Equal(const PBString& s1, const PBString& s2) {
  return s1 == s2 ? PBString::True() : PBString::False();
}

PBString Builtin_Print(const PBString& s) {
  constexpr int kBufferSize = MaxSizeNumChars() + 9;
  char pattern_buffer[kBufferSize];
  size_t length1, length2;
  s.GetLengths(length1, length2);
  const char* raw1;
  const char* raw2;
  s.GetRawStrings(raw1, raw2);
  snprintf(pattern_buffer, kBufferSize, "%%.%zus",  length1);
  printf(pattern_buffer, raw1);
  snprintf(pattern_buffer, kBufferSize, "%%.%zus",  length2);
  printf(pattern_buffer, raw2);
  printf("\n");
  return s;
}

PBString Builtin_Substring(
    const PBString& s, const PBString& start_str, const PBString& end_str) {
  size_t start, end;
  if (!start_str.StringToSize(start)) {
    start = 0;
  }
  if (!end_str.StringToSize(end)) {
    end = s.Length();
  }
  return PBString::Substring(s, start, end);
}
