#pragma once

enum class dirty_mode
{
	/**
	* \brief Fastest - mark as dirty on any assignment.
	*/
	on_assignment,
	/**
	* \brief Fast - always compare until marked as dirty.
	*/
	until_dirty
};

class dirty_impl
{
public:
	virtual ~dirty_impl() = default;
	virtual bool dirty() const = 0;
	virtual void clear() = 0;
	virtual void mark() = 0;
};

template <typename T, dirty_mode set_mode = dirty_mode::until_dirty>
class dirty_t
{
protected:
	T    _data {};
	bool _dirty = false;

public:
	dirty_t() = default;

	explicit dirty_t(const T& initial_value)
	{
		_data = initial_value;
	}

	bool dirty() const
	{
		return _dirty;
	}

	void clear()
	{
		_dirty = false;
	}

	void mark()
	{
		_dirty = true;
	}

	const T& data() const
	{
		return _data;
	}

	void data(const T& value)
	{
		assign(value);
	}

	dirty_t& operator=(const T& value)
	{
		assign(value);
		return *this;
	}

	operator const T&()
	{
		return data();
	}

protected:
	void assign(const T& value)
	{
		if constexpr (set_mode == dirty_mode::on_assignment)
		{
			_dirty = true;
		}
		else if constexpr (set_mode == dirty_mode::until_dirty)
		{
			if (!_dirty)
			{
				_dirty = value != _data;
			}
		}

		_data = value;
	}
};
