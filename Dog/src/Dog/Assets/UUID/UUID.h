#pragma once

namespace Dog {

	class UUID
	{
	public:
		UUID();
		UUID(uint64_t uuid);
		UUID(const UUID&) = default;
		UUID(const std::string& name);

		operator uint64_t() const { return mUUID; }

		bool operator==(const UUID& other) const { return mUUID == other.mUUID; }
		bool operator!=(const UUID& other) const { return mUUID != other.mUUID; }

		bool operator==(uint64_t other) const { return mUUID == other; }
		bool operator!=(uint64_t other) const { return mUUID != other; }

	private:
		uint64_t mUUID;
	};

}

namespace std {
	template <typename T> struct hash;

	template<>
	struct hash<Dog::UUID>
	{
		std::size_t operator()(const Dog::UUID& uuid) const
		{
			return (uint64_t)uuid;
		}
	};

}

