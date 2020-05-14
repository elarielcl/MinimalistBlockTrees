
#include "gtest/gtest.h"



class GTestFixture : public ::testing::Test {
protected:
    virtual void TearDown() {

    }

    virtual void SetUp() {

    }

public:

    GTestFixture() {
    }

    virtual ~GTestFixture() {
    }
};

TEST_F(GTestFixture, test) {

}