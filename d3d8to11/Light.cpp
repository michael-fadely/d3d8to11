#include "pch.h"
#include "Light.h"
#include "CBufferWriter.h"

void Light::copy(const D3DLIGHT8& rhs)
{
	this->type         = rhs.Type;
	this->diffuse      = float4(rhs.Diffuse.r, rhs.Diffuse.g, rhs.Diffuse.b, rhs.Diffuse.a);
	this->specular     = float4(rhs.Specular.r, rhs.Specular.g, rhs.Specular.b, rhs.Specular.a);
	this->ambient      = float4(rhs.Ambient.r, rhs.Ambient.g, rhs.Ambient.b, rhs.Ambient.a);
	this->position     = float3(rhs.Position.x, rhs.Position.y, rhs.Position.z);
	this->direction    = float3(rhs.Direction.x, rhs.Direction.y, rhs.Direction.z);
	this->range        = rhs.Range;
	this->falloff      = rhs.Falloff;
	this->attenuation0 = rhs.Attenuation0;
	this->attenuation1 = rhs.Attenuation1;
	this->attenuation2 = rhs.Attenuation2;
	this->theta        = rhs.Theta;
	this->phi          = rhs.Phi;
}

bool Light::operator==(const Light& rhs) const
{
	return enabled      == rhs.enabled &&
	       type         == rhs.type &&
	       diffuse      == rhs.diffuse &&
	       specular     == rhs.specular &&
	       ambient      == rhs.ambient &&
	       position     == rhs.position &&
	       direction    == rhs.direction &&
	       range        == rhs.range &&
	       falloff      == rhs.falloff &&
	       attenuation0 == rhs.attenuation0 &&
	       attenuation1 == rhs.attenuation1 &&
	       attenuation2 == rhs.attenuation2 &&
	       theta        == rhs.theta &&
	       phi          == rhs.phi;
}

bool Light::operator!=(const Light& rhs) const
{
	return !(*this == rhs);
}

CBufferBase& operator<<(CBufferBase& writer, const Light& l)
{
	return writer
		<< l.enabled
		<< l.type
		<< l.diffuse
		<< l.specular
		<< l.ambient
		<< l.position
		<< l.direction
		<< l.range
		<< l.falloff
		<< l.attenuation0
		<< l.attenuation1
		<< l.attenuation2
		<< l.theta
		<< l.phi;
}