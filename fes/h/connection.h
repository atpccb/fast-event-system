// design-patterns-cpp14 by Ricardo Marmolejo Garc�a is licensed under a Creative Commons
// Reconocimiento 4.0 Internacional License.
// http://creativecommons.org/licenses/by/4.0/
//
#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include <memory>
#include <fes/h/internal_connection.h>

namespace fes {

template <typename... Args>
using shared_connection = std::shared_ptr<internal_connection<Args...>>;
template <typename... Args>
using weak_connection = std::weak_ptr<internal_connection<Args...>>;

template <typename... Args>
class connection
{
public:
	connection() {}

	connection(const weak_connection<Args...>& other)
		: _connection(other)
	{
	}

	connection& operator=(const weak_connection<Args...>& other)
	{
		_connection = other;
		return *this;
	}

	~connection()
	{
		if (auto conn = _connection.lock())
		{
			conn->disconnect();
		}
	}

	connection(const connection&) = delete;
	connection(connection<Args...>&&) = delete;
	connection& operator=(const connection&) = delete;
	connection& operator=(connection&&) = delete;

protected:
	weak_connection<Args...> _connection;
};

}

#endif
