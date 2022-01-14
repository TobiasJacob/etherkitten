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

#include "InterfaceInfo.hpp"
#include <ifaddrs.h>
#include <set>
#include <utility>

namespace etherkitten::controller
{
	InterfaceInfo::InterfaceInfo()
	    : type(InterfaceType::GENERIC)
	    , socket(-1)
	{
	}

	InterfaceInfo::InterfaceInfo(int socket)
	    : InterfaceInfo()
	{
		type = InterfaceType::RAW_SOCKET;
		this->socket = socket;
	}

	InterfaceInfo::InterfaceInfo(bool withBusMock)
	    : InterfaceInfo()
	{
		if (withBusMock)
			type = InterfaceType::MOCKED;
	}

	InterfaceType InterfaceInfo::getType() { return type; }

	int InterfaceInfo::getSocket() { return socket; }

	std::vector<std::string> InterfaceInfo::getInterfaceNames()
	{
		switch (type)
		{
		case InterfaceType::RAW_SOCKET:
			return { "raw_socket: " + std::to_string(socket) };
		case InterfaceType::MOCKED:
			return { "mocked bus" };
		default:
			struct ifaddrs* ifaddr;
			if (getifaddrs(&ifaddr) == -1)
			{
				type = InterfaceType::MOCKED;
				return { "mocked bus (interfaces could not be acquired!)" };
			}

			std::set<std::string> nameSet;
			for (struct ifaddrs* ifa = ifaddr; ifa; ifa = ifa->ifa_next)
			{
				if (ifa->ifa_addr)
					nameSet.insert(ifa->ifa_name);
			}

			// ifaddrs might be listed multiple times
			std::vector<std::string> names;
			for (const std::string& name : nameSet)
			{
				names.push_back(name);
			}

			return names;
		}
	}
} // namespace etherkitten::controller
