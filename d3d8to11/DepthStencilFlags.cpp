#include "pch.h"

#include "DepthStencilFlags.h"

bool DepthStencilFlags::dirty() const
{
	return flags.dirty() || depth_flags.dirty() || stencil_flags.dirty();
}

void DepthStencilFlags::clear()
{
	flags.clear();
	depth_flags.clear();
	stencil_flags.clear();
}

void DepthStencilFlags::mark()
{
	flags.mark();
	depth_flags.mark();
	stencil_flags.mark();
}

bool DepthStencilFlags::operator==(const DepthStencilFlags& rhs) const
{
	return flags.data() == rhs.flags.data() &&
	       depth_flags.data() == rhs.depth_flags.data() &&
	       stencil_flags.data() == rhs.stencil_flags.data();
}
