/*****************************************************************//**
 * \file   FileHash.hpp
 * \author Adi (aditya.prakash@digipen.edu)
 * \date   October 29 2024
 * \Copyright @ 2024 Digipen (USA) Corporation * 
 
 * \brief  Utility for hashing files (currently used for model and texture files)
 *  *********************************************************************/

#pragma once

namespace Rendering
{
  class FileHash
  {
  public:
    static uint32_t ComputeFileHash(const std::string& filePath);
    static uint32_t LoadHash(const std::string& filePath);
  };
}