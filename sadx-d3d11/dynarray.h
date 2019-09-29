#pragma once

#include <cstdlib>
#include <cstring>

template <typename T>
class dynarray
{
protected:
	T* data_ = nullptr;
	size_t size_ = 0;

public:
	dynarray() = default;

	dynarray(size_t size)
	{
		resize(size);
	}

	dynarray(const dynarray& other)
	{
		*this = other;
	}

	dynarray(dynarray&& other) noexcept
	{
		*this = std::move(other);
	}

	~dynarray()
	{
		clear();
	}

	[[nodiscard]] T* data() const
	{
		return data_;
	}

	[[nodiscard]] size_t size() const
	{
		return size_;
	}

	[[nodiscard]] bool is_empty() const
	{
		return !size_;
	}

	void resize(size_t new_size)
	{
		auto old = data_;
		data_ = new T[new_size];

		if (size_ && old)
		{
			for (size_t i = 0; i < std::min(size_, new_size); i++)
			{
				data_[i] = std::move(old[i]);
			}
		}

		delete[] old;
		size_ = new_size;
	}

	void clear()
	{
		delete[] data_;
		size_ = 0;
	}

	// TODO: bounds check
	T& operator[](size_t i)
	{
		return data_[i];
	}

	// TODO: bounds check
	const T& operator[](size_t i) const
	{
		return data_[i];
	}

	dynarray& operator=(dynarray&& other) noexcept
	{
		data_ = other.data_;
		size_ = other.size_;

		other.data_ = nullptr;
		other.size_ = 0;

		return *this;
	}

	dynarray& operator=(const dynarray& other)
	{
		if (other.size())
		{
			resize(other.size());

			for (size_t i = 0; i < other.size(); i++)
			{
				data_[i] = std::move(other[i]);
			}
		}

		return *this;
	}

	class iterator;
	class const_iterator;

	iterator begin()
	{
		return iterator(data_);
	}

	iterator end()
	{
		return iterator(data_ + size_);
	}

	[[nodiscard]] const_iterator cbegin() const
	{
		return const_iterator(data_);
	}

	[[nodiscard]] const_iterator cend() const
	{
		return const_iterator(data_ + size_);
	}

	class iterator
	{
	public:
		using self_type         = iterator;
		using value_type        = T;
		using reference         = T&;
		using pointer           = T*;
		using difference_type   = ptrdiff_t;
		using iterator_category = size_t;

		iterator()
			: ptr_(nullptr)
		{
		}

		iterator(pointer ptr)
			: ptr_(ptr)
		{
		}

		self_type operator++()
		{
			++ptr_;
			return *this;
		}

		self_type operator++(int /*junk*/)
		{
			self_type i = *this;
			++ptr_;
			return i;
		}

		reference operator*() { return *ptr_; }
		pointer operator->() { return ptr_; }
		bool operator==(const self_type& rhs) const { return ptr_ == rhs.ptr_; }
		bool operator!=(const self_type& rhs) const { return ptr_ != rhs.ptr_; }

		self_type& operator=(const self_type& other)
		{
			ptr_ = other.ptr_;
			return *this;
		}

	private:
		pointer ptr_;
	};

	class const_iterator
	{
	public:
		using self_type         = const_iterator;
		using value_type        = T;
		using reference         = T&;
		using pointer           = T*;
		using difference_type   = ptrdiff_t;
		using iterator_category = size_t;

		const_iterator(pointer ptr)
			: ptr_(ptr)
		{
		}

		self_type operator++()
		{
			++ptr_;
			return *this;
		}

		self_type operator++(int /*junk*/)
		{
			self_type i = *this;
			++ptr_;
			return i;
		}

		reference operator*() const { return *ptr_; }
		pointer operator->() const { return ptr_; }
		bool operator==(const self_type& rhs) const { return ptr_ == rhs.ptr_; }
		bool operator!=(const self_type& rhs) const { return ptr_ != rhs.ptr_; }

		self_type& operator=(const self_type& other)
		{
			ptr_ = other.ptr_;
			return *this;
		}

	private:
		pointer ptr_;
	};
};

template <typename T>
auto begin(dynarray<T>& x)
{
	return x.begin();
}

template <typename T>
auto end(dynarray<T>& x)
{
	return x.end();
}
