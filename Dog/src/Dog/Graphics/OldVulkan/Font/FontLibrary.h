/*****************************************************************//**
 * \file   FontLibrary.hpp
 * \author Adi (aditya.prakash@digipen.edu)
 * \date   October 19 2024
 * \Copyright @ 2024 Digipen (USA) Corporation * 
 
 * \brief  Font library to store and manage fonts
 *  *********************************************************************/

#pragma once

#include "../Core/Device.h"
#include "Font.h"

namespace Dog
{
  // Forward Declaration
  class TextureLibrary;

  class FontLibrary
  {
  public:
    const static uint32_t INVALID_FONT_INDEX;
    const static uint32_t MAX_FONT_COUNT;

    FontLibrary(Device& device, TextureLibrary& textureLibrary);
    ~FontLibrary();

    FontLibrary(const FontLibrary&) = delete;
    FontLibrary& operator=(const FontLibrary&) = delete;

    const std::string& FontPathToTexturePath(const std::string& fontPath) const;

    uint32_t AddFont(const std::string& fontFilename);

    uint32_t GetFontIndex(const std::string& fontFilename);
    const Font& GetFont(const std::string& fontFilename);

    const Font& GetFontByIndex(uint32_t index) const { return *mFonts[index]; }

  private:
    Device& mDevice;
    TextureLibrary& mTextureLibrary;
    msdfgen::FreetypeHandle* mFreetypeHandle;

    std::vector<std::unique_ptr<Font>> mFonts;
    std::unordered_map<std::string, uint32_t> mFontMap;
  };
}