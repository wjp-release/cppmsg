#include "gtest/gtest.h"
#include "reactor.h"
#include <iostream>
#include <stdexcept>
#include <exception>

using namespace msg::posix;

class reactor_test : public ::testing::Test {
protected:
	reactor_test() {}
	virtual ~reactor_test() {}
	virtual void SetUp() {
	}
	virtual void TearDown() {
	}
};

TEST_F(reactor_test, basic) {
	
	

}


