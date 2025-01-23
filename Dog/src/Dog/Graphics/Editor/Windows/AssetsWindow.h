#pragma once

#ifndef DOG_SHIP

namespace Dog {

	class FileBrowser
	{
	public:
		FileBrowser();

		std::filesystem::path baseDir;
		std::filesystem::path currentDir;
	};

	void UpdateAssetsWindow(FileBrowser& browser/*, class TextEditorWrapper& textEditor*/);

}

#endif