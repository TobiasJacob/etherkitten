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

#include <array>

#include <etherkitten/datatypes/SlaveInfo.hpp>
#include <etherkitten/datatypes/dataobjects.hpp>
#include <etherkitten/datatypes/time.hpp>

namespace etherkitten::reader
{
	/*!
	 * \brief The BusInfo struct contains general information needed to control an EtherCAT bus.
	 */
	struct LogBusInfo
	{
		/*!
		 * \brief The actual size of the IOMap as used by SOEM.
		 *
		 * Everything beyond the index ioMapUsedSize - 1 is not read or written to,
		 * and the contents are undefined.
		 */
		size_t ioMapUsedSize;

		/*!
		 * \brief Contains the offsets for PDO objects relative to their slave starting point in the
		 * ioMap.
		 *
		 * SOEM automatically maps each slave's PDOs into the IOMap and tells us the starting point
		 * for each slave's inputs and outputs, but not which PDO is at which offset.
		 *
		 * This map contains exactly that information - the offset of an input / output PDO relative
		 * to its slave's input / output starting point.
		 */
		std::unordered_map<datatypes::PDO, datatypes::PDOInfo, datatypes::PDOHash,
		    datatypes::PDOEqual>
		    pdoOffsets;

		/*!
		 * \brief A TimeStamp that is earlier than all DataPoints.
		 *
		 * This TimeStamp can be used as the base time to calculate time offsets
		 * of DataPoints.
		 */
		datatypes::TimeStamp startTime;
	};
} // namespace etherkitten::reader
