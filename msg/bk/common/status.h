#pragma once

#include <string>

namespace msg{

/*
    IEEE Definitions
    Error: Human mistake that caused fault
    Fault: Discrepancy in code that causes a failure.
    Failure: External behavior is incorrect
*/

class status {
public:
    status() : state_(nullptr) {}
    ~status() { delete[] state_; }
    status(const status& s);
    void operator=(const status& s);
    static status success() { return status(); }
    static status error(const std::string& msg) {
        return status(StatusError, msg);
    }
    static status fault(const std::string& msg) {
        return status(StatusFault, msg);
    }
    static status failure(const std::string& msg) {
        return status(StatusFailure, msg);
    }
    static status unsupported(const std::string& msg) {
        return status(StatusUnsupported, msg);
    }
    bool is_success() const { return (state_ == nullptr); }
    bool is_error() const { return code() == StatusError; }
    bool is_fault() const { return code() == StatusFault; }
    bool is_failure() const { return code() == StatusFailure; }
    bool is_unsupported() const {return code() == StatusUnsupported; }
    std::string str() const;
 private:
    const char* state_;
    enum Code {
        StatusSuccess = 0,
        StatusError = 1,
        StatusFault = 2,
        StatusFailure = 3,
        StatusUnsupported = 4,
    };
    Code code() const {
        return (state_ == nullptr) ? StatusSuccess : static_cast<Code>(state_[4]);
    }
    status(Code code, const std::string& msg);
    static const char* CopyState(const char* s);
};

inline status::status(const status& s) {
  state_ = (s.state_ == nullptr) ? nullptr : CopyState(s.state_);
}
inline void status::operator=(const status& s) {
  if (state_ != s.state_) {
    delete[] state_;
    state_ = (s.state_ == nullptr) ? nullptr : CopyState(s.state_);
  }
}

}