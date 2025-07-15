#pragma once

#include "Core/ISystem.h"

namespace Dog
{
    class TestSystem : public ISystem
    {
    public:
        TestSystem() : ISystem("TestSystem") {};

    };
}
