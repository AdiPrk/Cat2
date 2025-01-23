/*****************************************************************//**
 * \file   Font.cpp
 * \author Adi (aditya.prakash@digipen.edu)
 * \date   October 19 2024
 * \Copyright @ 2024 Digipen (USA) Corporation * 
 
 * \brief  Generate MSDF font atlas and data
 *  *********************************************************************/

#include <PCH/pch.h>
#include "Font.h"
#include "../Texture/TextureLibrary.h"

namespace Dog
{
	Font::Font(msdfgen::FreetypeHandle* freetypeHandle, const std::string& fontFilename, TextureLibrary& textureLibrary)
		: mFreetypeHandle(freetypeHandle)
		, mTextureLibrary(textureLibrary)
	{
		std::string fileName = fontFilename.substr(0, fontFilename.find_last_of("."));
		mTexturePath = MSDF_FONT_DIR + fileName + "-msdf.png";

		GenerateMSDFAtlas(fontFilename);

		// add to tex lib
		mTextureLibrary.AddTexture(mTexturePath);

		// log glyph count
		const msdf_atlas::FontGeometry& fontGeom = GetFontGeometry();
		DOG_INFO("{0} Glyphs Loaded", fontGeom.getGlyphs().size());
	}

	Font::~Font()
	{
	}

	bool Font::GenerateMSDFAtlas(const std::string& fontFilename)
	{
		msdfgen::FontHandle* font = msdfgen::loadFont(mFreetypeHandle, ("assets/fonts/" + fontFilename).c_str());
		if (!font) return false;

		mFontGeometry = msdf_atlas::FontGeometry(&glyphs);
		mFontGeometry.loadCharset(font, 1.0, msdf_atlas::Charset::ASCII);

		const double maxCornerAngle = 3.0;
		for (msdf_atlas::GlyphGeometry& glyph : glyphs)
			glyph.edgeColoring(&msdfgen::edgeColoringInkTrap, maxCornerAngle, 0);

		msdf_atlas::TightAtlasPacker packer;

		packer.setDimensionsConstraint(msdf_atlas::DimensionsConstraint::SQUARE);
		packer.setMinimumScale(24.0);
		packer.setPixelRange(2.0);
		packer.setMiterLimit(1.0);
		packer.pack(glyphs.data(), static_cast<int>(glyphs.size()));

		mEmSize = static_cast<float>(packer.getScale());

		int atlasWidth = 0, atlasHeight = 0;
		packer.getDimensions(atlasWidth, atlasHeight);
		
		msdf_atlas::ImmediateAtlasGenerator<
			float, // pixel type of buffer for individual glyphs depends on generator function
			3, // number of atlas color channels
			msdf_atlas::msdfGenerator, // function to generate bitmaps for individual glyphs
			msdf_atlas::BitmapAtlasStorage<unsigned char, 3> // class that stores the atlas bitmap
			// For example, a custom atlas storage class that stores it in VRAM can be used.
		> generator(atlasWidth, atlasHeight);
		mAtlasWidth = static_cast<float>(atlasWidth);
		mAtlasHeight = static_cast<float>(atlasHeight);

		msdf_atlas::GeneratorAttributes attributes;
		generator.setAttributes(attributes);
		generator.setThreadCount(4);

		generator.generate(glyphs.data(), static_cast<int>(glyphs.size()));

		// save the atlas
		const msdfgen::BitmapConstRef<unsigned char, 3>& atlasBitmap = generator.atlasStorage();
		msdfgen::savePng(atlasBitmap, mTexturePath.c_str());

		// Precompute glyph data and store it in the map
		int i = 0;
		for (const msdf_atlas::GlyphGeometry& glyph : glyphs) 
		{
			// get glyph size
			double tl, tb, tr, tt;
			glyph.getQuadAtlasBounds(tl, tb, tr, tt);
			tl /= (double)atlasWidth; tb /= (double)atlasHeight; 
			tr /= (double)atlasWidth; tt /= (double)atlasHeight;


			GlyphData data;
			data.texInfo[0] = static_cast<float>(tl);
			data.texInfo[1] = static_cast<float>(tb);
			data.texInfo[2] = static_cast<float>(tr);
			data.texInfo[3] = static_cast<float>(tt);

			// Store the data in the map using the character code as the key
			mGlyphDataMap[glyph.getCodepoint()] = data;
			mGlyphBuffer.data[i] = data;
			++i;
		}

		msdfgen::destroyFont(font);

		return true;
	}
}
