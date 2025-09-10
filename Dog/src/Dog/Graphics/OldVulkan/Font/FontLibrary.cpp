/*****************************************************************//**
 * \file   FontLibrary.cpp
 * \author Adi (aditya.prakash@digipen.edu)
 * \date   October 19 2024
 * \Copyright @ 2024 Digipen (USA) Corporation * 
 
 * \brief  Font library to store and manage fonts
 *  *********************************************************************/

#include <PCH/pch.h>
#include "FontLibrary.h"
#include "../Texture/TextureLibrary.h"
#include <msdf-atlas-gen/msdf-atlas-gen.h>

namespace Dog
{
	const uint32_t FontLibrary::INVALID_FONT_INDEX = 9999;
	const uint32_t FontLibrary::MAX_FONT_COUNT = 1000;

	FontLibrary::FontLibrary(Device& device, TextureLibrary& textureLibrary)
		: mDevice(device)
		, mTextureLibrary(textureLibrary)
	{
		mFreetypeHandle = msdfgen::initializeFreetype();

		if (!mFreetypeHandle)
		{
			DOG_CRITICAL("Failed to initialize freetype!");
		}

		AddFont("Baloo-Regular.ttf");
	}

	FontLibrary::~FontLibrary()
	{
		msdfgen::deinitializeFreetype(mFreetypeHandle);
	}

	uint32_t FontLibrary::AddFont(const std::string& fontFilename)
	{
		// check size 
		if (mFonts.size() >= MAX_FONT_COUNT)
		{
			DOG_CRITICAL("Font count exceeded maximum");
			return INVALID_FONT_INDEX;
		}

		if (mFontMap.find(fontFilename) == mFontMap.end())
		{
			uint32_t fontIndex = static_cast<uint32_t>(mFonts.size());

            mFontMap[fontFilename] = fontIndex;
            mFonts.push_back(std::make_unique<Font>(mFreetypeHandle, fontFilename, mTextureLibrary));

			mTextureLibrary.AddTexture(mFonts.at(fontIndex)->GetTexturePath());

			return fontIndex;
		}
		else
		{
			return mFontMap[fontFilename];
		}
	}

	const Font& FontLibrary::GetFont(const std::string& fontFilename)
	{
		return GetFontByIndex(GetFontIndex(fontFilename));
	}

	uint32_t FontLibrary::GetFontIndex(const std::string& fontFilename)
	{
		if (mFontMap.find(fontFilename) != mFontMap.end())
		{
      return mFontMap[fontFilename];
    }
		else
		{
      return INVALID_FONT_INDEX;
    }
	}

	const std::string& FontLibrary::FontPathToTexturePath(const std::string& fontPath) const
	{
		return mFonts.at(mFontMap.at(fontPath))->GetTexturePath();
	}

}