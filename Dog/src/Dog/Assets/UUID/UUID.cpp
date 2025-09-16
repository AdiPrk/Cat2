#include <PCH/pch.h>
#include "UUID.h"

namespace Dog {

	static std::random_device s_RandomDevice;
	static std::mt19937_64 s_RandomEngine(s_RandomDevice());
	static std::uniform_int_distribution<uint64_t> s_UniformDistribution;

	UUID::UUID()
		: mUUID(s_UniformDistribution(s_RandomEngine))
	{

	}

	UUID::UUID(uint64_t uuid)
		: mUUID(uuid)
	{
	}

	UUID::UUID(const std::string& name)
	{
		mUUID = std::hash<std::string>{}(name);
	}

}
