#include "fepch.h"
#include "Renderer2D.h"
#include "VertexArray.h"
#include "Shader.h"
#include "RenderCommand.h"
#include "UniformBuffer.h"

#ifdef FE_WITH_VULKAN
#include "Platform/Vulkan/VulkanTextureRegistry.h"
#endif

#ifndef FE_PLATFORM_MACOS
#include "StorageBuffer.h"
#endif

#include <cstdio>
#include <glm/gtc/matrix_transform.hpp>


namespace FunEngine
{


	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;
		float TexIndex;
		float TilingFactor;

		// Editor-Only
		int EntityID = 0;
	};

	struct Renderer2DData
	{
		const uint32_t MaxQuads = 10000;
		const uint32_t MaxVertices = MaxQuads * 4;
		const uint32_t MaxIndices = MaxQuads * 6;
#ifdef FE_PLATFORM_MACOS
		// Apple GL ~4.1: fixed sampler2D array (no bindless / SSBO).
		static inline uint32_t MaxTextureSlots = 16;
#else
		static inline uint32_t MaxTextureSlots = 1000;
#endif

		Ref<VertexArray> QuadVertexArray;
		Ref<VertexBuffer> QuadVertexBuffer;
		Ref<Shader> TextureShader;
		Ref<Texture2D> WhiteTexture;

		uint32_t QuadIndexCount = 0;

		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;

		std::vector<Ref<Texture2D>> TextureSlots;
		std::vector<uint64_t> TextureHandles;
		uint32_t TextureSlotIndex = 1; // 0 = White Texture

		glm::vec4 QuadVertexPositions[4];

		Renderer2D::Statistics Stats;

		struct CameraData
		{
			glm::mat4 ViewProjection;
		};
		CameraData CameraBuffer;
		Ref<UniformBuffer> CameraUniformBuffer;
#ifndef FE_PLATFORM_MACOS
		Ref<StorageBuffer> TexturesBuffer = nullptr;
#endif
	};

	static Renderer2DData s_Data;

	static bool IsVulkanRenderer()
	{
		return RendererAPI::GetAPI() == RendererAPI::API::Vulkan;
	}

	// GL on macOS uses a fixed sampler array bound slot by slot; Vulkan uses the same slot table
	// but only binds the slots in use. GL elsewhere uses bindless handles in an SSBO.
	static bool UsesSlotTable()
	{
#ifdef FE_PLATFORM_MACOS
		return true;
#else
		return IsVulkanRenderer();
#endif
	}


	void Renderer2D::Init()
	{


		s_Data.QuadVertexArray = VertexArray::Create();


		s_Data.QuadVertexBuffer = VertexBuffer::Create(s_Data.MaxVertices * sizeof(QuadVertex));

		BufferLayout quadVBLayout = {
			{ShaderDataType::Float3, "a_Position"},
			{ShaderDataType::Float4, "a_Color"},
			{ShaderDataType::Float2, "a_TexCoord"},
			{ShaderDataType::Float, "a_TexIndex"},
			{ShaderDataType::Float, "a_TilingFactor"},
			{ShaderDataType::Int, "a_EntityID"}

		};

		s_Data.QuadVertexBuffer->SetLayout(quadVBLayout);

		s_Data.QuadVertexArray->AddVertexBuffer(s_Data.QuadVertexBuffer);

		s_Data.QuadVertexBufferBase = new QuadVertex[s_Data.MaxVertices];
		

		uint32_t* quadIndices = new uint32_t[s_Data.MaxIndices];

		uint32_t offset = 0;
		for (uint32_t i = 0; i < s_Data.MaxIndices; i+= 6)
		{
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;

			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;

			offset += 4;
		}

		
		FunEngine::Ref<IndexBuffer> quadIB = IndexBuffer::Create(quadIndices, s_Data.MaxIndices);

		s_Data.QuadVertexArray->SetIndexBuffer(quadIB);

		delete[] quadIndices;


		//s_Data.FlatColorShader = FunEngine::Shader::Create("assets/shaders/FlatColor.glsl");

		s_Data.WhiteTexture = FunEngine::Texture2D::Create(1, 1);

		uint32_t whiteTextureData = 0xffffffff;

		s_Data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		if (IsVulkanRenderer())
		{
#ifdef FE_WITH_VULKAN
			auto& registry = VulkanTextureRegistry::Get();
			Renderer2DData::MaxTextureSlots = registry.GetMaxTextureSlots();
			s_Data.TextureShader = FunEngine::Shader::Create(registry.UsesDescriptorIndexing()
				? "assets/shaders/vulkan/Renderer2D" : "assets/shaders/vulkan/Renderer2D_fixed");
			FE_CORE_INFO("Renderer2D (Vulkan): {0} texture slots ({1})", Renderer2DData::MaxTextureSlots,
				registry.UsesDescriptorIndexing() ? "descriptor indexing" : "fixed array");
#endif
		}
		else
		{
#ifdef FE_PLATFORM_MACOS
		// Prefer Texture_Mac.glsl (Editor); fall back to Texture.glsl (Sandbox #version 330).
		{
			FILE* f = fopen("assets/shaders/Texture_Mac.glsl", "r");
			if (f)
			{
				fclose(f);
				s_Data.TextureShader = FunEngine::Shader::Create("assets/shaders/Texture_Mac.glsl");
			}
			else
			{
				s_Data.TextureShader = FunEngine::Shader::Create("assets/shaders/Texture.glsl");
			}
		}
#else
		s_Data.TextureShader = FunEngine::Shader::Create("assets/shaders/Texture.glsl");
		s_Data.TextureHandles.emplace_back(s_Data.WhiteTexture->GetHandle());
#endif
		}

		s_Data.TextureShader->Bind();

#ifdef FE_PLATFORM_MACOS
		if (!IsVulkanRenderer())
		{
			std::vector<int32_t> samplers(Renderer2DData::MaxTextureSlots);
			for (uint32_t i = 0; i < Renderer2DData::MaxTextureSlots; i++)
				samplers[i] = (int32_t)i;
			s_Data.TextureShader->SetIntArray("u_Textures", samplers.data(), Renderer2DData::MaxTextureSlots);
		}
#endif

		if (UsesSlotTable())
		{
			s_Data.TextureSlots.resize(Renderer2DData::MaxTextureSlots);
			s_Data.TextureSlots[0] = s_Data.WhiteTexture;
		}
		else
		{
			s_Data.TextureSlots.emplace_back(s_Data.WhiteTexture);
		}

		s_Data.QuadVertexPositions[0] = {-0.5f, -0.5f, 0.0f, 1.0f};
		s_Data.QuadVertexPositions[1] = { 0.5f, -0.5f, 0.0f, 1.0f};
		s_Data.QuadVertexPositions[2] = { 0.5f,  0.5f, 0.0f, 1.0f};
		s_Data.QuadVertexPositions[3] = {-0.5f,  0.5f, 0.0f, 1.0f};

		s_Data.CameraUniformBuffer = UniformBuffer::Create(sizeof(Renderer2DData::CameraData), 0);
#ifndef FE_PLATFORM_MACOS
		if (!IsVulkanRenderer())
			s_Data.TexturesBuffer = StorageBuffer::Create(s_Data.MaxTextureSlots * sizeof(uint64_t), 1);
#endif
	}

	void Renderer2D::Shutdown()
	{
		delete[] s_Data.QuadVertexBufferBase;
		s_Data.QuadVertexBufferBase = nullptr;
		s_Data.QuadVertexBufferPtr = nullptr;

		s_Data.QuadVertexArray.reset();
		s_Data.QuadVertexBuffer.reset();
		s_Data.TextureShader.reset();
		s_Data.WhiteTexture.reset();
		s_Data.TextureSlots.clear();
		s_Data.CameraUniformBuffer.reset();
#ifndef FE_PLATFORM_MACOS
		s_Data.TexturesBuffer.reset();
#endif
	}

	void Renderer2D::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
		s_Data.CameraBuffer.ViewProjection = camera.GetProjectionMatrix() * glm::inverse(transform);
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer2DData::CameraData));

		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

		s_Data.QuadIndexCount = 0;

		s_Data.TextureSlotIndex = 1;
	}

	void Renderer2D::BeginScene(const EditorCamera& camera)
	{
		s_Data.CameraBuffer.ViewProjection = camera.GetViewProjection();
		s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer2DData::CameraData));

		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

		s_Data.QuadIndexCount = 0;

		s_Data.TextureSlotIndex = 1;
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		if (IsVulkanRenderer())
		{
			s_Data.CameraBuffer.ViewProjection = camera.GetViewProjectionMatrix();
			s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer, sizeof(Renderer2DData::CameraData));
		}
		else
		{
			s_Data.TextureShader->Bind();
			s_Data.TextureShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
		}

		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

		s_Data.QuadIndexCount = 0;

		s_Data.TextureSlotIndex = 1;
	/*	s_Data.TextureShader->Bind();
		s_Data.TextureShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());*/
		
	}


	void Renderer2D::Flush()
	{
		if (IsVulkanRenderer())
		{
			for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
				s_Data.TextureSlots[i]->Bind(i);
		}
#ifdef FE_PLATFORM_MACOS
		else
		{
			for (uint32_t i = 0; i < Renderer2DData::MaxTextureSlots; i++)
			{
				Ref<Texture2D> tex = (i < s_Data.TextureSlotIndex && s_Data.TextureSlots[i])
					? s_Data.TextureSlots[i] : s_Data.WhiteTexture;
				tex->Bind(i);
			}
		}
#else
		else
		{
			if (s_Data.TextureHandles.size() != s_Data.TextureSlots.size())
			{
				s_Data.TextureHandles.clear();
			}
			for (uint32_t i = 0; i < s_Data.TextureSlots.size(); i++)
			{
				if (s_Data.TextureHandles.size() != s_Data.TextureSlots.size())
				{
					s_Data.TextureHandles.emplace_back(s_Data.TextureSlots[i]->GetHandle());
				}
			}
			s_Data.TexturesBuffer->SetData((const void*) s_Data.TextureHandles.data(), s_Data.TextureHandles.size() * sizeof(uint64_t), 0);
		}
#endif

		s_Data.TextureShader->Bind();
		RenderCommand::DrawIndexed(s_Data.QuadVertexArray,s_Data.QuadIndexCount);
		s_Data.Stats.DrawCalls++;
	}

	void Renderer2D::EndScene()
	{
		uint32_t dataSize = (uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.QuadVertexBufferBase;

		s_Data.QuadVertexBuffer->SetData(s_Data.QuadVertexBufferBase, dataSize);

		Flush();
	}

	void Renderer2D::FlushAndReset()
	{
		EndScene();

		s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

		s_Data.QuadIndexCount = 0;

		s_Data.TextureSlotIndex = 1;
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{
		DrawQuad({ position.x, position.y , 0.0f }, size, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{


		constexpr size_t quadVertexCount = 4;
		const float textureIndex = 0.0f; // White Texture
		constexpr glm::vec2 textureCords[] = { {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };
		const float tilingFactor = 1.0f;

		if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
			FlushAndReset();

	

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		for (size_t i = 0; i < quadVertexCount; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Color = color;
			s_Data.QuadVertexBufferPtr->TexCoord = textureCords[i];
			s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
			s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;

			s_Data.QuadVertexBufferPtr++;
		}


		s_Data.QuadIndexCount += 6;

		s_Data.Stats.QuadCount++;

		
	/*	s_Data.TextureShader->SetFloat("u_TilingFactor", 1.0f);


		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		s_Data.TextureShader->SetMat4("u_Transform", transform);

		s_Data.WhiteTexture->Bind();

		s_Data.QuadVertexArray->Bind();

		RenderCommand::DrawIndexed(s_Data.QuadVertexArray);*/
	}
	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D> texture, float tilingFactor, const glm::vec4& color)
	{
		DrawQuad({ position.x, position.y , 0.0f }, size, texture, tilingFactor, color);
	}
	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D> texture, float tilingFactor, const glm::vec4& color)
	{


		constexpr size_t quadVertexCount = 4;
		//const float textureIndex = 0.0f; // White Texture
		constexpr glm::vec2 textureCords[] = { {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };


		if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
			FlushAndReset();

		float textureIndex = 0.0f;
		
		for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
		{
			if (*s_Data.TextureSlots[i].get() == *texture.get())
			{
				textureIndex = (float)i;
				break;
			}
		}
		
		if (textureIndex == 0.0f)
		{
			if (s_Data.TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
				FlushAndReset();
			textureIndex = (float)s_Data.TextureSlotIndex;
			s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
			s_Data.TextureSlotIndex++;
		}

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		for (size_t i = 0; i < quadVertexCount; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Color = color;
			s_Data.QuadVertexBufferPtr->TexCoord = textureCords[i];
			s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
			s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;

			s_Data.QuadVertexBufferPtr++;
		}

		s_Data.QuadIndexCount += 6;

		s_Data.Stats.QuadCount++;
	}



	void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color, int entityID)
	{


		constexpr size_t quadVertexCount = 4;
		const float textureIndex = 0.0f; // White Texture
		constexpr glm::vec2 textureCords[] = { {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };

		if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
			FlushAndReset();

		const float texIndex = 0.0f; // White Texture
		const float tilingFactor = 1.0f;

		for (size_t i = 0; i < quadVertexCount; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Color = color;
			s_Data.QuadVertexBufferPtr->TexCoord = textureCords[i];
			s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
			s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			s_Data.QuadVertexBufferPtr->EntityID = entityID;

			s_Data.QuadVertexBufferPtr++;
		}


		s_Data.QuadIndexCount += 6;

		s_Data.Stats.QuadCount++;
	}



	void Renderer2D::DrawQuad(const glm::mat4& transform, const Ref<Texture2D> texture, float tilingFactor, const glm::vec4& color, int entityID)
	{

		constexpr size_t quadVertexCount = 4;
		//const float textureIndex = 0.0f; // White Texture
		constexpr glm::vec2 textureCords[] = { {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };

		if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
			FlushAndReset();

		float textureIndex = 0.0f;

		if (UsesSlotTable())
		{
			for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
			{
				if (*s_Data.TextureSlots[i].get() == *texture.get())
				{
					textureIndex = (float)i;
					break;
				}
			}

			if (textureIndex == 0.0f)
			{
				if (s_Data.TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
					FlushAndReset();
				textureIndex = (float)s_Data.TextureSlotIndex;
				s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
				s_Data.TextureSlotIndex++;
			}
		}
		else
		{
			bool textureAlreadyBound = false;

			for (uint32_t i = 0; i < s_Data.TextureSlots.size() && !textureAlreadyBound; i++)
			{
				if (*s_Data.TextureSlots[i].get() == *texture.get())
				{
					textureIndex = (float)i;
					textureAlreadyBound = true;
				}
			}

			if (!textureAlreadyBound)
			{
				s_Data.TextureSlots.emplace_back(texture);
				textureIndex = s_Data.TextureSlots.size() - 1;
			}
		}


		for (size_t i = 0; i < quadVertexCount; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Color = color;
			s_Data.QuadVertexBufferPtr->TexCoord = textureCords[i];
			s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
			s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			s_Data.QuadVertexBufferPtr->EntityID = entityID;

			s_Data.QuadVertexBufferPtr++;
		}


		s_Data.QuadIndexCount += 6;

		s_Data.Stats.QuadCount++;
	}


	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, float rotation)
	{
		DrawRotatedQuad({position.x, position.y, 0.0f}, size, color, rotation);
	}


	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, float rotation)
	{

		constexpr size_t quadVertexCount = 4;
		const float textureIndex = 0.0f; // White Texture
		constexpr glm::vec2 textureCords[] = { {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };
		const float tilingFactor = 1.0f;

		if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
			FlushAndReset();


		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::rotate(glm::mat4(1.0f), glm::radians(rotation), {0.0f, 0.0f, 1.0f})
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		for (size_t i = 0; i < quadVertexCount; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Color = color;
			s_Data.QuadVertexBufferPtr->TexCoord = textureCords[i];
			s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
			s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;

			s_Data.QuadVertexBufferPtr++;
		}


		s_Data.QuadIndexCount += 6;

		s_Data.Stats.QuadCount++;

	}
	
	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D> texture, float rotation, float tilingFactor, const glm::vec4& color)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, texture, rotation, tilingFactor, color);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D> texture, float rotation, float tilingFactor, const glm::vec4& color)
	{


		constexpr size_t quadVertexCount = 4;
		//const float textureIndex = 0.0f; // White Texture
		constexpr glm::vec2 textureCords[] = { {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };

		if (s_Data.QuadIndexCount >= s_Data.MaxIndices)
			FlushAndReset();

		float textureIndex = 0.0f;

		for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
		{
			if (*s_Data.TextureSlots[i].get() == *texture.get())
			{
				textureIndex = (float)i;
				break;
			}
		}

		if (textureIndex == 0.0f)
		{
			textureIndex = (float)s_Data.TextureSlotIndex;
			s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
			s_Data.TextureSlotIndex++;
		}

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::rotate(glm::mat4(1.0f), glm::radians(rotation), {0.0f, 0.0f, 1.0f})
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		for (size_t i = 0; i < quadVertexCount; i++)
		{
			s_Data.QuadVertexBufferPtr->Position = transform * s_Data.QuadVertexPositions[i];
			s_Data.QuadVertexBufferPtr->Color = color;
			s_Data.QuadVertexBufferPtr->TexCoord = textureCords[i];
			s_Data.QuadVertexBufferPtr->TexIndex = textureIndex;
			s_Data.QuadVertexBufferPtr->TilingFactor = tilingFactor;

			s_Data.QuadVertexBufferPtr++;
		}

		s_Data.QuadIndexCount += 6;

		s_Data.Stats.QuadCount++;
	}


	void Renderer2D::DrawSprite(const glm::mat4& transform, SpriteRendererComponent& src, int entityID)
	{
		if (src.Texture)
			DrawQuad(transform, src.Texture, src.TilingFactor, src.Color, entityID);
		else
			DrawQuad(transform, src.Color, entityID);
	}



	void Renderer2D::ResetStats()
	{
		memset(&s_Data.Stats, 0, sizeof(Statistics));
	}

	Renderer2D::Statistics Renderer2D::GetStats()
	{
		return s_Data.Stats;
	}
}

