#include "stdafx.h"
#include "Material.h"
#include "CBufferWriter.h"

Material::Material(const D3DMATERIAL8& rhs)
{
	Diffuse  = float4(rhs.Diffuse.r, rhs.Diffuse.g, rhs.Diffuse.b, rhs.Diffuse.a);
	Ambient  = float4(rhs.Ambient.r, rhs.Ambient.g, rhs.Ambient.b, rhs.Ambient.a);
	Specular = float4(rhs.Specular.r, rhs.Specular.g, rhs.Specular.b, rhs.Specular.a);
	Emissive = float4(rhs.Emissive.r, rhs.Emissive.g, rhs.Emissive.b, rhs.Emissive.a);
	Power    = rhs.Power;
}

Material& Material::operator=(const D3DMATERIAL8& rhs)
{
	*this = Material(rhs);
	return *this;
}

bool Material::operator==(const Material& rhs) const
{
	return Diffuse  == rhs.Diffuse &&
		   Ambient  == rhs.Ambient &&
		   Specular == rhs.Specular &&
		   Emissive == rhs.Emissive &&
		   Power    == rhs.Power;
}

CBufferBase& operator<<(CBufferBase& writer, const Material& material)
{
	return writer
		   << material.Diffuse
		   << material.Ambient
		   << material.Specular
		   << material.Emissive
		   << material.Power;
}
