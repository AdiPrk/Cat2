/*****************************************************************//**
 * \file   Font.hpp
 * \author Adi (aditya.prakash@digipen.edu)
 * \date   October 19 2024
 * \Copyright @ 2024 Digipen (USA) Corporation * 
 
 * \brief  Font class which holds the font data and generates atlas
 *  *********************************************************************/

#pragma once

namespace Dog
{
#define MSDF_FONT_DIR "assets/fonts/msdf/"

  struct GlyphData {
    float texInfo[4];      // left, top, right, bottom
  };

  struct GlyphDataBuffer {
    static const int MAX_DRAW_COUNT = 95;

    std::array<GlyphData, MAX_DRAW_COUNT> data; // [MAX_DRAW_COUNT]
  };

  // forward declaration
  class TextureLibrary;

  class Font
  {
  public:
    Font(msdfgen::FreetypeHandle* freetypeHandle, const std::string& fontFilename, TextureLibrary& textureLibrary);
    ~Font();

    const std::string& GetTexturePath() const { return mTexturePath; }

    // get buffer
    const GlyphDataBuffer& GetGlyphBuffer() const { return mGlyphBuffer; }
    const GlyphData& GetGlyphData(char c) const { return mGlyphDataMap.at(c); }

    //const msdf_atlas::FontGeometry& GetFontGeometry() const { return mFontGeometry; }
    const msdf_atlas::FontGeometry& GetFontGeometry() const { return mFontGeometry; }

    float GetEmSize() const { return mEmSize; }
    float GetAtlasWidth() const { return mAtlasWidth; }
    float GetAtlasHeight() const { return mAtlasHeight; }
    
  private:
    // Generate font atlas and data
    bool GenerateMSDFAtlas(const std::string& fontFilename);

    // Map to store precomputed data for each glyph
    std::unordered_map<char, GlyphData> mGlyphDataMap;

    GlyphDataBuffer mGlyphBuffer;
    msdf_atlas::FontGeometry mFontGeometry;

    msdfgen::FreetypeHandle* mFreetypeHandle;
    std::string mTexturePath;

    std::vector<msdf_atlas::GlyphGeometry> glyphs = {};

    float mEmSize = 0.0f;
    float mAtlasWidth = 0.0f;
    float mAtlasHeight = 0.0f;

    // Texture library
    TextureLibrary& mTextureLibrary;
  };
}
