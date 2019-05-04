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

status get_posix_family(uint8_t f, sa_family_t& fam);

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
    addr(){
        memset(data, 0, 31);
    }
    addr(const addr& a): family(a.family){
        memcpy(data, a.data, 31);
    }
    addr& operator=(const addr& a){
        family=a.family;
        memcpy(data, a.data, 31);
        return *this;
    }
    void init(uint8_t fam, uint16_t port, sockaddr_in* sin);
    void init(uint8_t fam, uint16_t port, sockaddr_in6* sin);
    void init(uint8_t fam, uint16_t port, sockaddr_un* sun);
    uint16_t posix_family() const noexcept;
    void set_port(uint16_t p);
    uint16_t port() const;
    int to_addrv4(sockaddr_in* sin)const noexcept;
    int to_addrv6(sockaddr_in6* sin)const noexcept;
    int to_uds(sockaddr_un* sun) const noexcept;
    // return sockaddr len
    int to_posix(addr_posix* sp) const;
    std::string str() const;
    bool operator==(const addr &rhs) const;
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