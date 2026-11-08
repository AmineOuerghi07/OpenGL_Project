#pragma once
#include <filesystem>
#include "FunEngine/Renderer/Texture.h"

namespace FunEngine
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