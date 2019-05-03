#pragma once

#include <string>
#include <cstring>

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
    static status error(const std::string& msg, uint64_t val=0) {
        return status(StatusError, msg, val);
    }
    static status fault(const std::string& msg, uint64_t val=0) {
        return status(StatusFault, msg, val);
    }
    static status failure(const std::string& msg, uint64_t val=0) {
        return status(StatusFailure, msg, val);
    }
    static status unsupported(const std::string& msg, uint64_t val=0) {
        return status(StatusUnsupported, msg, val);
    }
    bool is_success() const { return (state_ == nullptr); }
    bool is_error() const { return code() == StatusError; }
    bool is_fault() const { return code() == StatusFault; }
    bool is_failure() const { return code() == StatusFailure; }
    bool is_unsupported() const {return code() == StatusUnsupported; }
    std::string str() const;
    uint64_t val() const{
        return reinterpret_cast<uint64_t>(state_);
    }
    uint32_t msglen() const{
        return strlen(state_+9);
    }
 private:
    // first 8 bytes: store uint64_t, could be a pointer or integral value
    // state_[9]: code
    // state_[10:]: msg that ends with '\0'
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
    status(Code code, const std::string& msg, uint64_t value);
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