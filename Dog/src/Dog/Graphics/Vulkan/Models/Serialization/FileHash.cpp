/*****************************************************************//**
 * \file   FileHash.cpp
 * \author Adi (aditya.prakash@digipen.edu)
 * \date   October 29 2024
 * \Copyright @ 2024 Digipen (USA) Corporation *

 * \brief  Implementation for file hashing
 *  *********************************************************************/

/*********************************************************************
 * Note to self:  Maybe check out other types of hashing like murmurhash3 later on
 *********************************************************************/

#include <PCH/pch.h>
#include "FileHash.hpp"

namespace Rendering
{
	// Fowler-Noll-Vo (FNV-1a) hash function
	uint32_t FileHash::ComputeFileHash(const std::string& filePath)
	{
		const uint32_t fnvPrime = 0x01000193U;    // FNV prime for 32-bit hash
		const uint32_t offsetBasis = 0x811C9DC5U; // FNV offset basis for 32-bit hash

		// Open file and get its size
		std::ifstream file(filePath, std::ios::binary | std::ios::ate);
		if (!file.is_open()) {
			//NL_CRITICAL("Failed to open file for hashing: {0}", filePath);
		}

		auto fileSize = file.tellg();
		file.seekg(0, std::ios::beg);

		std::vector<char> buffer(fileSize);
		file.read(buffer.data(), fileSize);

		uint32_t hash = offsetBasis;
		char previousChar = '\0';
		char currentChar;

		while (file.get(currentChar)) {
			if (currentChar == '\r') {
				continue; // Skip \r to handle \r\n as \n
			}

			// trimming redundant whitespace
			if ((currentChar == ' ' || currentChar == '\t') &&
				(previousChar == ' ' || previousChar == '\t' || previousChar == '\n' || previousChar == '\0')) {
				continue;
			}

			// Update previousChar for the next iteration
			previousChar = currentChar;

			// Apply FNV-1a hashing
			hash ^= static_cast<uint8_t>(currentChar);
			hash *= fnvPrime;
		}

		return hash;
	}

	uint32_t FileHash::LoadHash(const std::string& filePath)
	{
		std::ifstream hashFile(filePath);
		if (!hashFile) {
			return 0; // Return 0 if the file doesn't exist
		}
		
		uint32_t hash;
		hashFile.read(reinterpret_cast<char*>(&hash), sizeof(hash)); // Read the first uint32_t
		return hash;
	}
}
