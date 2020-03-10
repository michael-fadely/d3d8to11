#pragma once

#include <d3d11_1.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

template <typename T>
struct Shader
{
	ComPtr<T>        shader;
	ComPtr<ID3DBlob> blob;

	Shader() = default;
	~Shader() = default;

	Shader(Shader&& other) noexcept
	{
		*this = std::move(other);
	}

	Shader(const Shader& other)
	{
		*this = other;
	}

	Shader(ComPtr<T> shader, ComPtr<ID3DBlob> blob)
	{
		this->shader = shader;
		this->blob   = blob;
	}

	Shader& operator=(Shader&& other) noexcept
	{
		shader = std::move(other.shader);
		blob   = std::move(other.blob);
		return *this;
	}

	Shader& operator=(const Shader& other)
	{
		shader = other.shader;
		blob   = other.blob;
		return *this;
	}

	bool operator==(const Shader<T>& other) const
	{
		return shader == other.shader &&
		       blob == other.blob;
	}

	[[nodiscard]] bool has_value() const
	{
		return shader != nullptr;
	}
};

using VertexShader = Shader<ID3D11VertexShader>;
using PixelShader  = Shader<ID3D11PixelShader>;
using HullShader   = Shader<ID3D11HullShader>;
using DomainShader = Shader<ID3D11DomainShader>;
