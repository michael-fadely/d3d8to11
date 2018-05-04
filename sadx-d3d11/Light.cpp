#include "stdafx.h"
#include "Light.h"
#include "CBufferWriter.h"

Light::Light(const D3DLIGHT8& rhs)
{
	this->Type         = rhs.Type;
	this->Diffuse      = float4(rhs.Diffuse.r, rhs.Diffuse.g, rhs.Diffuse.b, rhs.Diffuse.a);
	this->Specular     = float4(rhs.Specular.r, rhs.Specular.g, rhs.Specular.b, rhs.Specular.a);
	this->Ambient      = float4(rhs.Ambient.r, rhs.Ambient.g, rhs.Ambient.b, rhs.Ambient.a);
	this->Position     = float3(rhs.Position.x, rhs.Position.y, rhs.Position.z);
	this->Direction    = float3(rhs.Direction.x, rhs.Direction.y, rhs.Direction.z);
	this->Range        = rhs.Range;
	this->Falloff      = rhs.Falloff;
	this->Attenuation0 = rhs.Attenuation0;
	this->Attenuation1 = rhs.Attenuation1;
	this->Attenuation2 = rhs.Attenuation2;
	this->Theta        = rhs.Theta;
	this->Phi          = rhs.Phi;
}

Light& Light::operator=(const D3DLIGHT8& rhs)
{
	*this = Light(rhs);
	return *this;
}

bool Light::operator==(const Light& rhs) const
{
	return Enabled      == rhs.Enabled &&
		   Type         == rhs.Type &&
		   Diffuse      == rhs.Diffuse &&
		   Specular     == rhs.Specular &&
		   Ambient      == rhs.Ambient &&
		   Position     == rhs.Position &&
		   Direction    == rhs.Direction &&
		   Range        == rhs.Range &&
		   Falloff      == rhs.Falloff &&
		   Attenuation0 == rhs.Attenuation0 &&
		   Attenuation1 == rhs.Attenuation1 &&
		   Attenuation2 == rhs.Attenuation2 &&
		   Theta        == rhs.Theta &&
		   Phi          == rhs.Phi;
}

bool Light::operator!=(const Light& rhs) const
{
	return !(*this == rhs);
}

CBufferBase& operator<<(CBufferBase& writer, const Light& l)
{
	return writer
		<< l.Enabled
		<< l.Type
		<< l.Diffuse
		<< l.Specular
		<< l.Ambient
		<< l.Position
		<< l.Direction
		<< l.Range
		<< l.Falloff
		<< l.Attenuation0
		<< l.Attenuation1
		<< l.Attenuation2
		<< l.Theta
		<< l.Phi;
}