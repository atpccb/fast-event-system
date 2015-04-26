// design-patterns-cpp14 by Ricardo Marmolejo García is licensed under a Creative Commons Reconocimiento 4.0 Internacional License.
// http://creativecommons.org/licenses/by/4.0/
//
#ifndef _FAST_EVENT_SYSTEM_
#define _FAST_EVENT_SYSTEM_

#include <functional>
#include <string>
#include <memory>
#include <map>
#include <exception>
#include <thread>
#include <chrono>
#include <deque>
#include <algorithm>
#include <iostream>
#include <tuple>
#include <functional>
#include <vector>
#include <list>
#include <string>
#include <sstream>
#include <queue>
#include <fast-event-system/common.h>

#if (__cplusplus == 201103) || ((__clang__ == 1) && (__clang_minor__ != 6))
namespace std
{
	template <bool B, typename T = void> using enable_if_t = typename std::enable_if<B, T>::type;
	
	template<typename T, typename... Args>
	std::unique_ptr<T> make_unique(Args&&... args)
	{
		return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
	}
}
#endif
#ifdef _MSC_VER
#define noexcept _NOEXCEPT
#endif

namespace fes {

template <typename ... Args>
class method;

template <typename ... Args>
class internal_connection
{
public:
	internal_connection(const std::function<void(void)>& deleter)
		: _deleter(deleter)
	{
		
	}
	
	internal_connection(const internal_connection<Args...>& other) = delete;
	const internal_connection<Args...>& operator=(const internal_connection<Args...>& other) = delete;
	
	void disconnect()
	{
		// call deleter
		_deleter();
	}

	~internal_connection()
	{
		
	}
	
protected:
	std::function<void(void)> _deleter;
};
template <typename ... Args> using shared_connection = std::shared_ptr<internal_connection<Args...> >;

template <typename ... Args>
class connection
{
public:
	connection()
	{
		
	}

	connection(const shared_connection<Args ...>& other)
		: _connection(other)
	{
		
	}
	
	const connection<Args...>& operator=(const shared_connection<Args ...>&other)
	{
		_connection = other;
		return *this;
	}

	~connection()
	{
		if (_connection)
		{
			_connection->disconnect();
		}
	}

protected:
	shared_connection<Args ...> _connection;
};

template <typename ... Args>
class method
{
public:
	method(const std::function<void(const Args&...)>& method)
		: _method(method)
	{
		
	}

	template <typename T>
	method(T* obj, void (T::*ptr_func)(const Args&...))
		: method(obj, ptr_func, make_int_sequence<sizeof...(Args)>{})
	{
		
	}

	template <typename T, int ... Is>
	method(T* obj, void (T::*ptr_func)(const Args&...), int_sequence<Is...>)
		: method(std::bind(ptr_func, obj, placeholder_template<Is>{}...))
	{
		
	}

	void operator()(const Args&... data)
	{
		_method(data...);
	}
		
protected:
	std::function<void(const Args&...)> _method;
};

template <typename ... Args>
class callback
{
public:
	using list_methods = std::list<method<Args...> >;
	
	callback()
	{
		
	}
	
	template <typename T>
	inline shared_connection<Args...> connect(T* obj, void (T::*ptr_func)(const Args&...))
	{
		return _connect(obj, ptr_func, make_int_sequence < sizeof...(Args) > {});
	}
	
	inline shared_connection<Args...> connect(const std::function<void(const Args&...)>& method)
	{
		auto it = _registered.emplace(_registered.end(), method);
		return std::make_shared<internal_connection<Args ...> >([&](){
			_registered.erase(it);
		});
	}

	void operator()(const Args& ... data)
	{
		for(auto& reg : _registered)
		{
			reg(data...);
		}
	}

protected:	
	template <typename T, int ... Is>
	inline shared_connection<Args...> _connect(T* obj, void (T::*ptr_func)(const Args&...), int_sequence<Is...>)
	{
		auto it = _registered.emplace(_registered.end(), std::bind(ptr_func, obj, placeholder_template<Is>{}...));
		return std::make_shared<internal_connection<Args ...> >([&](){
			_registered.erase(it);
		});
	}
	
protected:
	list_methods _registered;
};

template <typename ... Args>
class message
{
public:
	message(int priority, std::chrono::system_clock::time_point timestamp, const Args&... data)
		: _priority(priority)
		, _timestamp(timestamp)
		, _data(data...)
	{
		
	}

	message(message<Args...>&& other) noexcept
		: _priority(other._priority)
		, _timestamp(other._timestamp)
		, _data(std::move(other._data))
	{
		
	}

	message<Args...>& operator=(message<Args...>&& other) noexcept
	{
		_priority = other._priority;
		_timestamp = other._timestamp;
		_data = std::move(other._data);

		return *this;
	}

	message(const message<Args...>& other)
		: _priority(other._priority)
		, _timestamp(other._timestamp)
		, _data(other._data)
	{

	}
	
	message<Args...>& operator=(const message<Args...>& other)
	{
		_priority = other._priority;
		_timestamp = other._timestamp;
		_data = other._data;

		return *this;
	}
	
	~message()
	{
		
	}

	bool operator<(const message<Args...>& other) const
	{
		if (_timestamp < other._timestamp)
			return false;
		else if (_timestamp > other._timestamp)
			return true;
		
		return (_priority < other._priority);
	}
	
	int _priority;
	std::chrono::system_clock::time_point _timestamp;
	std::tuple<Args...> _data;
};

template <typename ... Args>
class queue_delayer
{
public:
	using container_type = std::priority_queue<message<Args...>, std::deque<message<Args...> > >;
	
	queue_delayer()
	{
		
	}

	template <typename R, typename P>
	void operator()(int priority, std::chrono::duration<R,P> delay, const Args& ... data)
	{
		auto delay_point = std::chrono::high_resolution_clock::now() + delay;
		_queue.emplace(priority, delay_point, data...);
	}
	
	void update()
	{
		while(!_queue.empty())
		{
			_dispatch();
		}
	}

	bool dispatch()
	{
		if(!_queue.empty())
		{
			return _dispatch();
		}
		return false;
	}
	
	const typename container_type::value_type& top() const
	{
		return _queue.top();
	}
	
	bool empty() const
	{
		return _queue.empty();
	}
	
	size_t size() const
	{
		return _queue.size();
	}

	template <typename T>
	inline shared_connection<Args...> connect(T* obj, void (T::*ptr_func)(const Args&...))
	{
		return _output.connect(obj, ptr_func);
	}

	inline shared_connection<Args...> connect(const std::function<void(const Args&...)>& method)
	{
		return _output.connect(method);
	}
protected:
	template<int ...S>
	inline void dispatch(const typename container_type::value_type& top, seq<S...>)
	{
		_output(std::get<S>(top._data)...);
	}

	bool _dispatch()
	{
		auto t1 = std::chrono::high_resolution_clock::now();
		auto& top = _queue.top();
		if (t1 >= top._timestamp)
		{
			dispatch(top, gens < sizeof...(Args) > {});
			_queue.pop();
			return true;
		}
		return false;
	}

protected:
	callback<Args... > _output;
	container_type _queue;
};

template <typename ... Args>
class queue_fast
{
public:
	using container_type = std::queue<std::tuple<Args...>, std::deque<std::tuple<Args...> > >;
	
	queue_fast()
	{
		
	}
	
	void operator()(const Args& ... data)
	{
		_queue.emplace(data...);
	}
	
	void update()
	{
		while(!_queue.empty())
		{
			_dispatch();
		}
	}

	bool dispatch()
	{
		if(!_queue.empty())
		{
			_dispatch();
			return true;
		}
		return false;
	}

	bool empty() const
	{
		return _queue.empty();
	}

	size_t size() const
	{
		return _queue.size();
	}

	template <typename T>
	inline shared_connection<Args...> connect(T* obj, void (T::*ptr_func)(const Args&...))
	{
		return _output.connect(obj, ptr_func);
	}

	inline shared_connection<Args...> connect(const std::function<void(const Args&...)>& method)
	{
		return _output.connect(method);
	}

protected:	
	template<int ...S>
	inline void dispatch(const typename container_type::value_type& top, seq<S...>)
	{
		_output(std::get<S>(top)...);
	}

	void _dispatch()
	{
		dispatch(_queue.front(), gens < sizeof...(Args) > {});
		_queue.pop();
	}

protected:
	callback<Args...> _output;
	container_type _queue;
};

} // end namespace

#endif
