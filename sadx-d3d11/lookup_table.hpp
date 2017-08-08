/**
 * Copyright (C) 2015 Patrick Mours. All rights reserved.
 * License: https://github.com/crosire/d3d8to9#license
 */

#pragma once

#include <unordered_map>
#include <algorithm>

template <typename T>
struct AddressCacheIndex;

class AddressLookupTable
{
public:
	explicit AddressLookupTable(Direct3DDevice8* Device);
	         ~AddressLookupTable();

	template <typename T>
	T* FindAddress(void* pAddress9)
	{
		if (pAddress9 == nullptr)
		{
			return nullptr;
		}

		auto it = AddressCache[AddressCacheIndex<T>::CacheIndex].find(pAddress9);

		if (it != std::end(AddressCache[AddressCacheIndex<T>::CacheIndex]))
		{
			return static_cast<T *>(it->second);
		}

		return new T(Device, static_cast<AddressCacheIndex<T>::Type9*>(pAddress9));
	}

	template <typename T>
	void SaveAddress(T* pAddress8, void* pAddress9)
	{
		if (pAddress8 == nullptr || pAddress9 == nullptr)
		{
			return;
		}

		AddressCache[AddressCacheIndex<T>::CacheIndex][pAddress9] = pAddress8;
	}

	template <typename T>
	void DeleteAddress(T* pAddress8)
	{
		if (pAddress8 == nullptr)
		{
			return;
		}

		constexpr UINT CacheIndex = AddressCacheIndex<T>::CacheIndex;

		auto it = std::find_if(
			AddressCache[CacheIndex].begin(), AddressCache[CacheIndex].end(),
			[pAddress8](std::pair<void*, class AddressLookupTableObject*> Map) -> bool
			{
				return Map.second == pAddress8;
			});

		if (it != std::end(AddressCache[CacheIndex]))
		{
			it = AddressCache[CacheIndex].erase(it);
		}
	}

private:
	Direct3DDevice8* const Device;
	std::unordered_map<void*, class AddressLookupTableObject*> AddressCache[8];
};

class AddressLookupTableObject
{
public:
	virtual ~AddressLookupTableObject()
	{
	}

	void DeleteMe()
	{
		delete this;
	}
};
