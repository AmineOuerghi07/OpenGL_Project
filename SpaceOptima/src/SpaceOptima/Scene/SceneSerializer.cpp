#include "sopch.h"

#include "SceneSerializer.h"


#include "Entity.h"
#include "Components.h"


#include <fstream>

#include <yaml-cpp/yaml.h>


namespace YAML {

	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};


}


namespace SpaceOptima
{

	static void SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		out << YAML::BeginMap;

		out << YAML::Key << "Entity" << YAML::Value << entity.GetID();
		if (entity.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap;

			auto& tag = entity.GetComponent<TagComponent>().Tag;

			out << YAML::Key << "Tag" << YAML::Value << tag;


			out << YAML::EndMap;
		}

		if (entity.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap;

			auto& tc = entity.GetComponent<TransformComponent>();

			out << YAML::Key << "Translation" << YAML::Value << YAML::convert<glm::vec3>::encode(tc.Translation);
			out << YAML::Key << "Rotation" << YAML::Value << YAML::convert<glm::vec3>::encode(tc.Rotation);
			out << YAML::Key << "Scale" << YAML::Value << YAML::convert<glm::vec3>::encode(tc.Scale);

			out << YAML::EndMap;
		}

		if (entity.HasComponent<CameraComponent>())
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap;

			auto& camera = entity.GetComponent<CameraComponent>();

			out << YAML::Key << "Camera";
			out << YAML::BeginMap;
			out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.Camera.GetProjectionType();
			out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.Camera.GetPerspectiveVerticalFOV();
			out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.Camera.GetPerspectiveNearClip();
			out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.Camera.GetPerspectiveFarClip();
			out << YAML::Key << "OrthographicSize" << YAML::Value << camera.Camera.GetOrthographicSize();
			out << YAML::Key << "OrthographicNear" << YAML::Value << camera.Camera.GetOrthographicNearClip();
			out << YAML::Key << "OrthographicFar" << YAML::Value << camera.Camera.GetOrthographicFarClip();
			out << YAML::EndMap;

			out << YAML::Key << "Primary" << YAML::Value << camera.Primary;
			out << YAML::Key << "FixedAspectRatio" << YAML::Value << camera.FixedAspectRatio;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<SpriteRendererComponent>())
		{
			out << YAML::Key << "SpriteRendererComponent";
			out << YAML::BeginMap;

			auto& spriteRendererComponent = entity.GetComponent<SpriteRendererComponent>();
			out << YAML::Key << "Color" << YAML::Value << YAML::convert<glm::vec4>::encode(spriteRendererComponent.Color);

			out << YAML::EndMap;
		}


		out << YAML::EndMap;
	}


	SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
		:m_Scene(scene)
	{
	}

	void SceneSerializer::Serialize(const std::string& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << "Untitled";
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
		auto view = m_Scene->m_Registry.view<entt::entity>();
		for (auto& entityID : view)
		{

			Entity entity = { entityID, m_Scene.get() };

			if (!entity)
				return;

			SerializeEntity(out, entity);




			//Entity entity = { entityID, m_Scene.get() };
			//if (!entity)
			//	return;

			//out << YAML::BeginMap; // Entity
			//out << YAML::Key << "Entity" << YAML::Value << entity.GetID(); // TODO: Entity ID Goes here


			//if (entity.HasComponent<TagComponent>())
			//{
			//	out << YAML::Key << "TagComponent";
			//	out << YAML::BeginMap; // Tag Component

			//	auto& tag = entity.GetComponent<TagComponent>().Tag;
			//	out << YAML::Key << "Tag" << YAML::Value << tag;

			//	YAML::EndMap; // Tag Component
			//}
			//
			//out << YAML::EndMap; // Entity
			
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	void SceneSerializer::SerializeRuntime(const std::string& filepath)
	{
	}

	bool SceneSerializer::Deserialize(const std::string& filepath)
	{
		std::ifstream stream(filepath);
		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());
		if (!data["Scene"])
			return false;

		std::string sceneName = data["Scene"].as<std::string>();

		auto entities = data["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				uint64_t uuid = entity["Entity"].as<uint64_t>(); // TODO UUID

				std::string name;
				auto tagComponent = entity["TagComponent"];
				if (tagComponent)
				{
					name = tagComponent["Tag"].as<std::string>();
				}

				Entity deserializedEntity = m_Scene->CreateEntity(name);

				auto transformComponent = entity["TransformComponent"];
				if (transformComponent)
				{
					auto& tc = deserializedEntity.GetComponent<TransformComponent>();

					tc.Translation = transformComponent["Translation"].as<glm::vec3>();
					tc.Rotation = transformComponent["Rotation"].as<glm::vec3>();
					tc.Scale = transformComponent["Scale"].as<glm::vec3>();
				}

				auto cameraComponent = entity["CameraComponent"];
				if (cameraComponent)
				{
					auto& camera = deserializedEntity.AddComponent<CameraComponent>();

					const auto& cameraProps = cameraComponent["Camera"];
					camera.Camera.SetProjectionType((SceneCamera::ProjectionType)cameraProps["ProjectionType"].as<int>());
					camera.Camera.SetPerspectiveVerticalFOV(cameraProps["PerspectiveFOV"].as<float>());

					camera.Camera.SetPerspectiveNearClip(cameraProps["PerspectiveNear"].as<float>());

					camera.Camera.SetPerspectiveFarClip(cameraProps["PerspectiveFar"].as<float>());

					camera.Camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());

					camera.Camera.SetOrthographicNearClip(cameraProps["OrthographicNear"].as<float>());

					camera.Camera.SetOrthographicFarClip(cameraProps["OrthographicFar"].as<float>());

					camera.Primary = cameraComponent["Primary"].as<bool>();

					camera.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
				}

				auto spriteRenderer = entity["SpriteRendererComponent"];
				if (spriteRenderer)
				{
					auto& src = deserializedEntity.AddComponent<SpriteRendererComponent>();

					src.Color = spriteRenderer["Color"].as<glm::vec4>();
				}
			}
		}

		return true;
	}

	bool SceneSerializer::DeserializeRuntime(const std::string& filepath)
	{
		return false;
	}

}