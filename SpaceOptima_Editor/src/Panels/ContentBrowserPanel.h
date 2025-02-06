#pragma once
#include <filesystem>
#include "SpaceOptima/Renderer/Texture.h"

namespace SpaceOptima
{
	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();
		void OnImGuiRender();
	private:
		std::filesystem::path m_CurrentDirectory;
		Ref<Texture2D> m_FileIcon;
		Ref<Texture2D> m_DirectoryIcon;
	};
}