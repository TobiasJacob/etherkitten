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


#include "topology.hpp"

#include <limits>

namespace etherkitten::reader::bSInformant
{
	std::vector<std::array<unsigned int, 4>> readBusTopology()
	{
		const unsigned int maxUInt = std::numeric_limits<unsigned int>::max();
		std::vector<std::array<unsigned int, 4>> topology(
		    ec_slavecount, { maxUInt, maxUInt, maxUInt, maxUInt });

		// Index 0 in ec_slave is the master, which does not have defined values for the topology.
		for (int slave = 1; slave <= ec_slavecount; ++slave)
		{
			// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
			ec_slavet& slaveInfo = ec_slave[slave];

			// parentport is the port on the parent connected to the slave
			topology[slaveInfo.parent][slaveInfo.parentport] = slave;

			// entryport is the port on the slave connected to the parent
			topology[slave][slaveInfo.entryport] = slaveInfo.parent;
		}

		return topology;
	}
} // namespace etherkitten::reader::bSInformant
