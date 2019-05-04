#include "addr.h"

namespace msg{

status get_posix_family(uint8_t f, sa_family_t& fam){
	switch(f) {
	case family_v4:
		fam = AF_INET;
		break;
	case family_v6:
		fam = AF_INET6;
		break;
	case family_uds:
		fam = AF_UNSPEC;
		break;
	default:
		return status::error("invalid family input");
	}
    return status::success();
}

void addr::init(uint8_t fam, uint16_t port, sockaddr_in* sin)
{
    family=fam;
    set_port(port);
    memcpy(data+2, &sin->sin_addr.s_addr, 4);
}

void addr::init(uint8_t fam, uint16_t port, sockaddr_in6* sin)
{
    family=fam;
    set_port(port);
    memcpy(data+2, sin->sin6_addr.s6_addr, 16);
}

void addr::init(uint8_t fam, uint16_t port, sockaddr_un* sun)
{
    family=fam;
    set_port(port);
    memcpy(data, sun->sun_path, 31);
}

uint16_t addr::posix_family() const noexcept{
    switch(family){
    case family_v4:
        return PF_INET;
    case family_v6:
        return PF_INET6;
    case family_uds:
        return PF_UNIX;
    default:
        return PF_INET;
    }
}

void addr::set_port(uint16_t p){
    if(family==family_uds) throw std::runtime_error("unix domain socket does not require a port");
    *reinterpret_cast<uint16_t*>(data)=p;
}

uint16_t addr::port() const{
    if(family==family_uds) throw std::runtime_error("unix domain socket does not require a port");
    return *reinterpret_cast<const uint16_t*>(data);
}

int addr::to_addrv4(sockaddr_in* sin)const noexcept{
    memset(sin, 0, sizeof(*sin));
    sin->sin_family = PF_INET;
    sin->sin_port = port();
    sin->sin_addr.s_addr = *reinterpret_cast<const uint32_t*>(data+2);
    return sizeof(*sin);        
}

int addr::to_addrv6(sockaddr_in6* sin)const noexcept{
    memset(sin, 0, sizeof(*sin));
    sin->sin6_family = PF_INET6;
    sin->sin6_port = port();
    memcpy(sin->sin6_addr.s6_addr, data+2, 16);
    return sizeof(*sin);        
}

int addr::to_uds(sockaddr_un* sun) const noexcept{
    memset(sun, 0, sizeof(*sun)); // note that we must memset 32 other than 
    memcpy(sun->sun_path, data, 31);
    sun->sun_family = PF_UNIX;
    return sizeof(*sun);
}

// return sockaddr len
int addr::to_posix(addr_posix* sp) const{
    switch(family){
    case family_v4:
        return to_addrv4(reinterpret_cast<sockaddr_in*>(sp));
    case family_v6:
        return to_addrv6(reinterpret_cast<sockaddr_in6*>(sp));
    case family_uds:
        return to_uds(reinterpret_cast<sockaddr_un*>(sp));
    default:
        throw std::runtime_error("invalid family");
    }
}

std::string addr::str() const{
    return std::string(reinterpret_cast<const char*>(this), 32);
}

bool addr::operator==(const addr &rhs) const { 
    if(family!=rhs.family){
        return false;
    }
    int len;
    switch(family){
    case family_v4:
        len=6;
        break;
    case family_v6:
        len=18;
        break;
    case family_uds:
        len=31;
        break;
    default:
        throw std::runtime_error("invalid family");
    }
    for(int i=0;i<len;i++){
        if(data[i] == rhs.data[i]) continue;
        return false;
    }
    return true; 
}

}