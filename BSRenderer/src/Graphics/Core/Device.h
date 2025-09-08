#pragma once

namespace BSR
{
    class Device
    {
    public:
        Device();
        ~Device();

    private:
        vkb::Device device;
    };
}
