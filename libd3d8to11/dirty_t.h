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

template <typename T, dirty_mode set_mode = dirty_mode::continuous>
class dirty_t
{
protected:
	T    m_last {};
	T    m_data {};
	bool m_dirty = false;

public:
	dirty_t() = default;

	explicit dirty_t(const T& initial_value)
	{
		m_data = initial_value;
	}

	[[nodiscard]] bool dirty() const
	{
		return m_dirty;
	}

	void clear()
	{
		m_dirty = false;

		if constexpr (set_mode == dirty_mode::continuous)
		{
			m_last = m_data;
		}
	}

	void mark()
	{
		m_dirty = true;
	}

	[[nodiscard]] const T& data() const
	{
		return m_data;
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
			m_dirty = true;
		}
		else if constexpr (set_mode == dirty_mode::until_dirty)
		{
			if (!m_dirty)
			{
				m_dirty = value != m_data;
			}
		}
		else if constexpr (set_mode == dirty_mode::continuous)
		{
			m_dirty = m_last != value;
		}

		m_data = value;
	}
};
