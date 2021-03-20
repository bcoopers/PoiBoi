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

#ifndef POIBOIC_ERROR_CODE_H_
#define POIBOIC_ERROR_CODE_H_

#include <optional>
#include <string>

namespace pbc {

class ErrorCode {
 public:
  struct Success{};

  struct Failure {
    explicit Failure(std::string msg) : error_msg_(msg) {}
    std::string error_msg_;
  };

  ErrorCode(ErrorCode::Success s) : success_(true) {}
  ErrorCode(ErrorCode::Failure f) : success_(false), error_msg_(std::move(f.error_msg_)) {}

  bool IsSuccess() const { return success_; }

  bool IsFailure() const { return !success_; }

  const std::string& ErrorMessage() const { return error_msg_; }


 private:
  bool success_ = false;
  std::string error_msg_;
};

template<typename T>
class ErrorOr {
 public:
  ErrorOr(T t) : item_(std::move(t)) {}

  ErrorOr(ErrorCode::Failure f) : error_msg_(std::move(f.error_msg_)) {}

  bool IsSuccess() const { return item_.has_value(); }

  bool IsFailure() const { return !IsSuccess(); }

  T& GetItem() { return item_.value(); }

  const T& GetItem() const { return item_.value(); }

  const std::string& ErrorMessage() const { return error_msg_; }

 private:
  ErrorOr() {}
  std::optional<T> item_;
  std::string error_msg_;
};

#define RETURN_EC_IF_FAILURE(ec) {  \
  const auto& local_ec = (ec); \
  if (local_ec.IsFailure()) { \
    return ErrorCode::Failure(local_ec.ErrorMessage()); \
  } \
}

}  // namespace pbc

#endif  // #ifndef POIBOIC_ERROR_CODE_H_
