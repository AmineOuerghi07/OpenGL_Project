#include "sopch.h"
#include "ContentBrowserPanel.h"

#include <imgui.h>

#include <filesystem>

namespace SpaceOptima
{
	extern const std::filesystem::path s_AssetsDirectory = "assets";

	ContentBrowserPanel::ContentBrowserPanel()
		:m_CurrentDirectory(s_AssetsDirectory)
	{
		m_FileIcon = Texture2D::Create("Resources/Icons/file.png");
		m_DirectoryIcon = Texture2D::Create("Resources/Icons/folder.png");
	}

	void ContentBrowserPanel::OnImGuiRender()
	{
		
		ImGui::Begin("Content Browser");

		if (m_CurrentDirectory != std::filesystem::path(s_AssetsDirectory))
		{
			if (ImGui::Button("<-"))
			{
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
			}
		}


		static float iconSize = 100.0f + 16.0f;
		
		//static float padding = 16.0f;

		float widthAvailable = ImGui::GetContentRegionAvail().x;
		int iconsNumber = (int)(widthAvailable / iconSize);
		if (iconsNumber < 1)
			iconsNumber = 1;
		
		ImGui::Columns(iconsNumber, 0, false);
		
		for (auto& p : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			const auto& path = p.path();
			std::string fileName = p.path().filename().string();
			ImGui::PushID(fileName.c_str());

			Ref<Texture2D> icon = p.is_directory() ? m_DirectoryIcon : m_FileIcon;
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::ImageButton(fileName.c_str(), (ImTextureID)icon->GetRendererID(), { iconSize, iconSize }, { 1 , 1 }, { 0 , 0 });
			
			if (ImGui::BeginDragDropSource())
			{
				std::filesystem::path relativePath(path);
				const wchar_t* itemPath = relativePath.c_str();
				ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));
				ImGui::EndDragDropSource();
			}
			
			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (p.is_directory())
					m_CurrentDirectory /= path.filename();

			}
			ImGui::TextWrapped(fileName.c_str());

			ImGui::NextColumn();

			ImGui::PopID();
			
		}

		ImGui::Columns(1);

		ImGui::End();
	}
}