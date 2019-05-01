#include "gtest/gtest.h"
#include "timer.h"
#include "clock.h"
#include <iostream>
#include <stdexcept>
#include <exception>

using namespace msg::posix;

class timer_test : public ::testing::Test {
protected:
	timer_test() {}
	virtual ~timer_test() {}
	virtual void SetUp() {
	}
	virtual void TearDown() {
	}
};

TEST_F(timer_test, basic) {
	timer t1;
	int duh_val=0;
	auto duh_op=[&]{duh_val++;};
	try{
		t1.push(duh_op, (1000));
		t1.push(duh_op, (200));
		t1.push(duh_op, (400));
		t1.push(duh_op, (700));
		t1.push(duh_op, (900));
		t1.push(duh_op, (600));
		t1.push(duh_op, (1400));
		EXPECT_EQ(t1.size(), 7);
		EXPECT_EQ(t1.pop().expire, 200);
		EXPECT_EQ(t1.pop().expire, 400);
		EXPECT_EQ(t1.pop().expire, 600);
		EXPECT_EQ(t1.pop().expire, 700);
		EXPECT_EQ(t1.pop().expire, 900);
		EXPECT_EQ(t1.pop().expire, 1000);
		EXPECT_EQ(t1.pop().expire, 1400);
	}catch(const std::runtime_error& e){
		std::cerr<<e.what()<<std::endl;
	}

}


