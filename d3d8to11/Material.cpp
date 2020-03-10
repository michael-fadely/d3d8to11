#include "stdafx.h"
#include "Material.h"
#include "CBufferWriter.h"

Material::Material(const D3DMATERIAL8& rhs)
{
	diffuse  = float4(rhs.Diffuse.r, rhs.Diffuse.g, rhs.Diffuse.b, rhs.Diffuse.a);
	ambient  = float4(rhs.Ambient.r, rhs.Ambient.g, rhs.Ambient.b, rhs.Ambient.a);
	specular = float4(rhs.Specular.r, rhs.Specular.g, rhs.Specular.b, rhs.Specular.a);
	emissive = float4(rhs.Emissive.r, rhs.Emissive.g, rhs.Emissive.b, rhs.Emissive.a);
	power    = rhs.Power;
}

Material& Material::operator=(const D3DMATERIAL8& rhs)
{
	*this = Material(rhs);
	return *this;
}

bool Material::operator==(const Material& rhs) const
{
	return diffuse  == rhs.diffuse &&
	       ambient  == rhs.ambient &&
	       specular == rhs.specular &&
	       emissive == rhs.emissive &&
	       power    == rhs.power;
}

CBufferBase& operator<<(CBufferBase& writer, const Material& material)
{
	return writer
	       << material.diffuse
	       << material.ambient
	       << material.specular
	       << material.emissive
	       << material.power;
}
