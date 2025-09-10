#pragma once

#include "ISystem.h"

namespace Dog
{
    class TestSystem : public ISystem
    {
    public:
        TestSystem() : ISystem("TestSystem") {};

    };
}
