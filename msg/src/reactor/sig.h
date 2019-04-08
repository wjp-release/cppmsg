#pragma once

namespace msg{namespace reactor{

class event;
event* create_signalfd_event(int epollfd);

}}