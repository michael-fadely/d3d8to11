/**
 * Copyright (C) 2015 Patrick Mours. All rights reserved.
 * License: https://github.com/crosire/d3d8to9#license
 */

#include "stdafx.h"
#include "d3d8to9.hpp"
#include "lookup_table.hpp"

AddressLookupTable::AddressLookupTable(Direct3DDevice8 *Device) :
	Device(Device)
{
	// Do nothing
}
AddressLookupTable::~AddressLookupTable()
{
	for (auto& i : AddressCache)
	{
		while (!i.empty())
		{
			auto it = i.begin();

			it->second->DeleteMe();

			it = i.erase(it);
		}
	}
}
