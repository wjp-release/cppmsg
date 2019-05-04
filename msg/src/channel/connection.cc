#include "connection.h"

namespace msg{

connection::connection(int fd):c(std::make_unique<conn>(fd)){}

}