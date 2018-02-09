#pragma once

#include <gtest/gtest.h>

class TestLNK : public ::testing::Test
{
public:
  virtual void SetUp();
  virtual void TearDown();
};
