/*
 * Copyright 2021 Niklas Arlt, Matthias Becht, Florian Bossert, Marwin Madsen, and Philip Scherer
 *
 * This file is part of EtherKITten.
 *
 * EtherKITten is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * EtherKITten is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with EtherKITten.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <optional>
#include <string>
#include <vector>

namespace etherkitten::controller
{
	enum class InterfaceType
	{
		GENERIC,
		RAW_SOCKET,
		MOCKED
	};

	/**
	 * \brief InterfaceInfo contains the information about how the bus might be accessed.
	 * \details This may be either over generic network interfaces, the raw_socket tool or an
	 * internal bus mock
	 */
	class InterfaceInfo
	{
	public:
		/**
		 * \brief Create an InterfaceInfo. The resulting object will indicate that generic network
		 * interfaces should be used to access the bus.
		 */
		InterfaceInfo();
		/**
		 * \brief Create an InterfaceInfo. The resulting object will indicate that input from
		 * raw_socket should be used to access the bus.
		 */
		InterfaceInfo(int socket);
		/**
		 * \brief Create an InterfaceInfo. The resulting object will indicate that the internal bus
		 * mock should be used instead of a real bus.
		 */
		InterfaceInfo(bool withBusMock);

		/**
		 * \brief Return the type of this InterfaceInfo
		 * \return the type of this InterfaceInfo
		 */
		InterfaceType getType();

		/**
		 * \brief Return the socket (from raw_socket) that the bus might be accessed over. The
		 * returned value is only meaningful if the return value of getType() is
		 * InterfaceType::RAW_SOCKET!
		 * \return the socket (from raw_socket) that the bus might be
		 * accessed over
		 */
		int getSocket();

		/**
		 * \brief Return names for the interfaces that the bus might be accessed over.
		 * \return names for the interfaces that the bus might be accessed over
		 */
		std::vector<std::string> getInterfaceNames();

	private:
		InterfaceType type;
		int socket;
	};
} // namespace etherkitten::controller
