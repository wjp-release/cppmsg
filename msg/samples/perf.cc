#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "system/reactor.h"
#include "system/timer.h"
#include "common/clock.h"
#include "channel/pipe.h"
#include "channel/connection.h"
#include "channel/basic_connection.h"
#include "session/session.h"
#include "sample.h"
#include <functional>
#include "system/resolv.h"
#include <sstream>
#include "system/resolv.h"
using namespace std;
using namespace msg;

#define DEFAULT_PORT 23456
#define DEFAULT_IP "localhost"

// Note that addr should be IP rather than url for cppmsg
static void latency_server(const char *addr, size_t msgsize, int trips);
static void latency_client(const char* addr, size_t msgsize, int trips);
static void latency_client_async(const char* addr, size_t msgsize, int trips);
static void throughput_server(const char *addr, size_t msgsize, int count);
static void throughput_client(const char *addr, size_t msgsize, int count);

static void die(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
	exit(2);
}

static int
parse_int(const char *arg, const char *what)
{
	long  val;
	char *eptr;

	val = strtol(arg, &eptr, 10);
	// Must be a postive number less than around a billion.
	if ((val < 0) || (val > (1 << 30)) || (*eptr != 0) || (eptr == arg)) {
		die("Invalid %s", what);
	}
	return ((int) val);
}

void do_local_lat(int argc, char **argv)
{
	long int msgsize;
	long int trips;

	if (argc != 3) {
		die("Usage: local_lat <listen-addr> <msg-size> <roundtrips>");
	}

	msgsize = parse_int(argv[1], "message size");
	trips   = parse_int(argv[2], "round-trips");

	latency_server(argv[0], msgsize, trips);
}

void do_remote_lat(int argc, char **argv)
{
	int msgsize;
	int trips;
	if (argc != 3) {
		die("Usage: remote_lat <connect-to> <msg-size> <roundtrips>");
	}
	msgsize = parse_int(argv[1], "message size");
	trips   = parse_int(argv[2], "round-trips");

	latency_client(argv[0], msgsize, trips);
}

void do_remote_lat_async(int argc, char **argv)
{
	int msgsize;
	int trips;
	if (argc != 3) {
		die("Usage: remote_lat <connect-to> <msg-size> <roundtrips>");
	}
	msgsize = parse_int(argv[1], "message size");
	trips   = parse_int(argv[2], "round-trips");

	latency_client_async(argv[0], msgsize, trips);
}

void do_local_thr(int argc, char **argv)
{
	int msgsize;
	int trips;

	if (argc != 3) {
		die("Usage: local_thr <listen-addr> <msg-size> <count>");
	}

	msgsize = parse_int(argv[1], "message size");
	trips   = parse_int(argv[2], "count");

	throughput_server(argv[0], msgsize, trips);
}

void do_remote_thr(int argc, char **argv)
{
	int msgsize;
	int trips;

	if (argc != 3) {
		die("Usage: remote_thr <connect-to> <msg-size> <count>");
	}

	msgsize = parse_int(argv[1], "message size");
	trips   = parse_int(argv[2], "count");

	throughput_client(argv[0], msgsize, trips);
}

void latency_client(const char* IP, size_t msgsize, int trips){
    //[0] Kicks off eventloop
    reactor::reactor::instance().start_eventloop();
    //[1] Resolve address synchronously
    addr address;
    auto s=resolv_ipv4_sync(address, IP, DEFAULT_PORT);
    if(!s.is_success()) logerr(s.str().c_str());
    //[2] Create a tcp connection fd
    int connfd;
    s=sync_connect(address,connfd);
    if(!s.is_success()) logerr(s.str().c_str());
    //[3] Create a cppmsg basic connection 
    auto c=basic_connection::make(connfd);
    //[4] Create a message
    message msg;
    msg.alloc(msgsize);
    //[5] Send and receive multiple messages.
    auto start=now();
    for(int i=0;i<trips;i++){
        c->sendmsg_async("");

    }



}

void latency_client_async(const char* IP, size_t msgsize, int trips){
    //[0] Kicks off eventloop
    reactor::reactor::instance().start_eventloop();
    //[1] Resolve address synchronously
    addr address;
    auto s=resolv_ipv4_sync(address, IP, DEFAULT_PORT);
    if(!s.is_success()) logerr(s.str().c_str());
    //[2] Create a tcp connection fd
    int connfd;
    s=sync_connect(address,connfd);
    if(!s.is_success()) logerr(s.str().c_str());
    //[3] Create a cppmsg basic connection 
    auto c=basic_connection::make(connfd);
    //[4] Create a message
    message msg;
    msg.alloc(msgsize);
    message recv;
    //[5] Send multiple messages asynchronously and collect their replies more aggressively
    auto start=now();
    for(int i=0;i<trips;i++){
        c->sendmsg_async(msg);
    }
    for(int i=0;recv.nr_chunks()<=trips && i<trips;i++){
        c->recv_multipart_msg(recv);
    }
    auto end=now();

    auto total   = (float) ((end - start)) / 1000;
	auto latency = ((float) ((total * 1000000)) / (trips * 2));
	printf("total time: %.3f [s]\n", total);
	printf("message size: %d [B]\n", (int) msgsize);
	printf("round trip count: %d\n", trips);
	printf("average latency: %.3f [us]\n", latency);
}

void latency_server(const char *addr, size_t msgsize, int trips)
{

}

void throughput_server(const char *addr, size_t msgsize, int count){

}

void throughput_client(const char *addr, size_t msgsize, int count){

}




// perf implements the same performance tests found in the standard nng, nanomsg & mangos performance tests. 
// Options are:
// - remote_lat - remote latency side (client, aka latency_client)
// - local_lat  - local latency side (server, aka latency_server)
// - local_thr  - local throughput side
// - remote_thr - remote throughput side
bool matches(const char *arg, const char *name)
{
	const char *ptr = arg;
	const char *x;

	while (((x = strchr(ptr, '/')) != NULL) ||
	    ((x = strchr(ptr, '\\')) != NULL) ||
	    ((x = strchr(ptr, ':')) != NULL)) {
		ptr = x + 1;
	}
	for (;;) {
		if (*name == '\0') {
			break;
		}
		if (tolower(*ptr) != *name) {
			return (false);
		}
		ptr++;
		name++;
	}
	switch (*ptr) {
	case '\0':
		return (true);
	case '.': // extension; ignore it.
		return (true);
	default: // some other trailing bit.
		return (false);
	}
}

int
main(int argc, char **argv)
{
	char *prog;
	// Allow -m <remote_lat> or whatever to override argv[0].
	if ((argc >= 3) && (strcmp(argv[1], "-m") == 0)) {
		prog = argv[2];
		argv += 3;
		argc -= 3;
	} else {
		prog = argv[0];
		argc--;
		argv++;
	}
	if (matches(prog, "remote_lat") || matches(prog, "latency_client")) {
		do_remote_lat(argc, argv);
	} else if (matches(prog, "local_lat") ||
	    matches(prog, "latency_server")) {
		do_local_lat(argc, argv);
	} else if (matches(prog, "local_thr") ||
	    matches(prog, "throughput_server")) {
		do_local_thr(argc, argv);
	} else if (matches(prog, "remote_thr") ||
	    matches(prog, "throughput_client")) {
		do_remote_thr(argc, argv);
	} else if(matches(prog, "remote_lat_async") ||
        matches(prog, "latency_client_async")){
        do_remote_lat_async(argc, argv);
    } 
    
    else {
		die("Unknown program mode? Use -m <mode>.");
	}
}