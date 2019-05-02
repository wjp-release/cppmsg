#include "status.h"
#include "def.h"

namespace msg{

const char* status::CopyState(const char* state) {
    uint32_t size;
    memcpy(&size, state, sizeof(size));
    char* result = new char[size + 5];
    memcpy(result, state, size + 5);
    return result;
}

status::status(Code code, const std::string& msg) {
    assert(code != StatusSuccess);
    const uint32_t len1 = msg.size();
    const uint32_t size = len1;
    char* result = new char[size + 5];
    memcpy(result, &size, sizeof(size));
    result[4] = static_cast<char>(code);
    memcpy(result + 5, msg.data(), len1);
    state_ = result;
}

std::string status::str() const {
  if (state_ == NULL) {
    return "OK";
  } else {
    char tmp[30];
    const char* type;
    switch (code()) {
      case StatusSuccess:
        type = "Success";
        break;
      case StatusError:
        type = "Human mistake: ";
        break;
      case StatusFault:
        type = "Discrepancy in code: ";
        break;
      case StatusFailure:
        type = "External behavior is incorrect: ";
        break;
      case StatusUnsupported:
        type = "Not supported yet: ";
        break;
      default:
        snprintf(tmp, sizeof(tmp), "Unknown code(%d): ",
                 static_cast<int>(code()));
        type = tmp;
        break;
    }
    std::string result(type);
    uint32_t length;
    memcpy(&length, state_, sizeof(length));
    result.append(state_ + 5, length);
    return result;
  }
}

}