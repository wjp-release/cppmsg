#pragma once

/*
struct sockaddr { //16 
   unsigned short    sa_family;    // address family, AF_xxx
   char              sa_data[14];  // 14 bytes of protocol address
};
struct sockaddr_in { //16
    short            sin_family;   // e.g. AF_INET, AF_INET6
    unsigned short   sin_port;     // e.g. htons(3490)
    struct in_addr   sin_addr;     // see struct in_addr, below
    char             sin_zero[8];  // zero this if you want to
}; // detailed sockaddr
struct sockaddr_in6 { //28B
    u_int16_t       sin_family;   // address family, AF_INET6
    u_int16_t       sin_port;     // port number, Network Byte Order
    u_int32_t       sin_flowinfo; // IPv6 flow information
    struct in6_addr sin_addr;     // IPv6 address
    u_int32_t       sin_scope_id; // Scope ID
};
struct sockaddr_storage { //128B
    sa_family_t  ss_family;     // address family
    // all this is padding, implementation specific, ignore it:
    char      __ss_pad1[_SS_PAD1SIZE];
    int64_t   __ss_align;
    char      __ss_pad2[_SS_PAD2SIZE];
}; // fit in both a struct sockaddr_in and struct sockaddr_in6
struct sockaddr_un {
    sa_family_t sun_family;               // AF_UNIX
    char        sun_path[108];            // Pathname 
};
*/

#include "def.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

namespace msg{

enum sockaddr_family: uint8_t{
    family_v4 = 0,
    family_v6 = 1,
    family_uds = 2
};

//addr_posix is a storage of posix sockaddr, large enough to fit in all three types above.
class addr_posix{ 
public:
    sockaddr* sa(){
        return reinterpret_cast<sockaddr*>(data);
    }
    char data[128]; 
};

// addr is a logical storage of sockaddr, which can be converted from and to addr_posix or any standard sockaddr struct.
class addr{
public:
    uint16_t posix_family() const noexcept{
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
    void set_port(uint16_t p){
        if(family==family_uds) throw std::runtime_error("unix domain socket does not require a port");
        *reinterpret_cast<uint16_t*>(data)=p;
    }
    uint16_t port() const{
        if(family==family_uds) throw std::runtime_error("unix domain socket does not require a port");
        return *reinterpret_cast<const uint16_t*>(data);
    }
    int to_addrv4(sockaddr_in* sin)const noexcept{
        memset(sin, 0, sizeof(*sin));
        sin->sin_family = PF_INET;
        sin->sin_port = port();
        sin->sin_addr.s_addr = *reinterpret_cast<const uint32_t*>(data+2);
        return sizeof(*sin);        
    }
    int to_addrv6(sockaddr_in6* sin)const noexcept{
        memset(sin, 0, sizeof(*sin));
        sin->sin6_family = PF_INET6;
        sin->sin6_port = port();
        memcpy(sin->sin6_addr.s6_addr, data+2, 16);
        return sizeof(*sin);        
    }
    int to_uds(sockaddr_un* sun) const noexcept{
		memset(sun, 0, sizeof(*sun)); // note that we must memset 32 other than 
        memcpy(sun->sun_path, data, 31);
		sun->sun_family = PF_UNIX;
        return sizeof(*sun);
    }
    // return sockaddr len
    int to_posix(addr_posix* sp) const{
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
    std::string str(){
        return std::string(reinterpret_cast<char*>(this), 32);
    }
    bool operator==(const addr &rhs) const { 
        if(lhs.family!=rhs.family){
            return false;
        }
        int len;
        switch(lhs.family){
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
            if(lhs.data[i] == rhs.data[i]) continue;
            return false;
        }
        return true; 
    }
    //sockaddr_family: family_v4, family_v6, family_uds
    uint8_t     family;
    // for v4, v6, addr[0], addr[1] stores port number
    // for v4, addr[2~5] stores addr (addr size=4)
    // for v6, addr[2~17] stores addr (addr size=16)
    // for uds, addr[0~30] stores uds path (max path len=31) 
    uint8_t     data[31]; 
};

struct addrcmp{
    bool operator()(const addr& lhs, const addr& rhs) const { 
        if(lhs.family!=rhs.family){
            return (uint8_t)lhs.family < (uint8_t)rhs.family;
        }
        int len;
        switch(lhs.family){
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
            if(lhs.data[i] == rhs.data[i]) continue;
            return lhs.data[i]<rhs.data[i];
        }
        return false; 
    }
};
 
}

namespace std {
    template<> struct hash<msg::addr> {
        std::size_t operator()(const msg::addr &f) const {
            return std::hash<string>{}(f.str());
        }  
    };
}