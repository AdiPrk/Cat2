/*****************************************************************//**
 * \file   ModelSerializer.hpp
 * \author Adi (aditya.prakash@digipen.edu)
 * \date   October 27 2024
 * \Copyright @ 2024 Digipen (USA) Corporation * 
 
 * \brief  Serializes model data to our custom model file format
 *  *********************************************************************/

#pragma once

#include "../Model.h"

namespace Dog {

  // Forward declarations
  class Device;
  class TextureLibrary;
  class MaterialSetLibrary;

  // Class for serializing and deserializing models
  class ModelSerializer {
  public:
    // Save a model to a file in little-endian format

    /*********************************************************************
     * param:  model: Model to save
     * param:  filename: File to save to
     * param:  hash: The model file's hash
     * 
     * brief:  Save the model to a .nlm file! Refer to ModelFileFormat.md for details on the file format.
     *********************************************************************/
    static void save(const Model& model, const std::string& filename, uint32_t hash);

    /*********************************************************************
     * param:  model: Model to save
     * param:  file: File to save to
     * 
     * brief:  Continues the 'save' function. Saves all the animation data.
     *********************************************************************/
    static void saveAnimationData(const Model& model, std::ofstream& file);

    /*********************************************************************
     * param:  model: Model to load
     * param:  filename: File to load from
     * 
     * brief:  Loads a model from a .nlm file! Refer to ModelFileFormat.md for details on the file format.
     *********************************************************************/
    static bool load(Model& model, const std::string& filename);

    static void loadAnimationData(Model& model, std::ifstream& file);


    static const std::string NL_MODEL_FILE_PATH;
    static const std::string NL_MODEL_EXTENTION;

  private:
    // Magic number for format verification
    static constexpr uint32_t MAGIC_NUMBER = 0x4D4F444C; // 'MODL'
    static constexpr uint32_t VERSION = 2;

    // Check if the system is little-endian
    static constexpr const bool switchEndian = std::endian::native != std::endian::little;

    // Utilities for swapping endianness
    static constexpr uint32_t swapEndian(uint32_t val);
    static float swapEndianFloat(float val);
    static glm::vec2 swapEndianVec2(const glm::vec2& vec);
    static glm::vec3 swapEndianVec3(const glm::vec3& vec);
    static glm::mat4 swapEndianMat4(const glm::mat4& mat);

    // Validate the file header and version
    static bool validateHeader(std::ifstream& file);
  };

} // namespace Rendering