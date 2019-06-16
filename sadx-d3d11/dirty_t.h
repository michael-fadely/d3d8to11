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
	until_dirty,
	/**
	 * \brief Slowest - check upon each assignment, even if marked dirty.
	 */
	continuous
};

class dirty_impl
{
public:
	virtual ~dirty_impl() = default;
	[[nodiscard]] virtual bool dirty() const = 0;
	virtual void clear() = 0;
	virtual void mark() = 0;
};

template <typename T, dirty_mode set_mode, typename enable = void>
class last_t;

template <typename T, dirty_mode set_mode>
class last_t<T, set_mode, std::enable_if_t<set_mode == dirty_mode::continuous>>
{
protected:
	T _last {};
};

template <typename T, dirty_mode set_mode>
class last_t<T, set_mode, std::enable_if_t<set_mode != dirty_mode::continuous>>
{
protected:
	// nothing
};

template <typename T, dirty_mode set_mode = dirty_mode::continuous>
class dirty_t : public last_t<T, set_mode>
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

	[[nodiscard]] bool dirty() const
	{
		return _dirty;
	}

	void clear()
	{
		_dirty = false;

		if constexpr (set_mode == dirty_mode::continuous)
		{
			_last = _data;
		}
	}

	void mark()
	{
		_dirty = true;
	}

	[[nodiscard]] const T& data() const
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
		else if constexpr (set_mode == dirty_mode::continuous)
		{
			_dirty = _last != value;
		}

		_data = value;
	}
};
