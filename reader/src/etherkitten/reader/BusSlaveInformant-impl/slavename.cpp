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


#include "slavename.hpp"

#include <algorithm>
#include <cstring>

#include <etherkitten/datatypes/dataobjects.hpp>

#include "impl-common.hpp"

namespace etherkitten::reader::bSInformant
{
	std::string getSlaveName(int slave) { return ec_slave[slave].name; } // NOLINT

	std::string getSlaveName(int slave, const CoEResult& coes)
	{
		constexpr unsigned int manufacturerSlaveNameIndex = 0x1008;
		auto it = std::find_if(
		    coes.first.begin(), coes.first.end(), [](const datatypes::CoEEntry& entry) {
			    return entry.getIndex() == manufacturerSlaveNameIndex;
		    });
		if (it == coes.first.end())
		{
			return ec_slave[slave].name; // NOLINT
		}

		const datatypes::CoEObject& nameObject = it->getObjects()[0];
		const size_t nameObjectBitLength = coes.second.at(nameObject).bitLength;
		static constexpr size_t byteSize = 8;
		size_t byteLength = nameObjectBitLength / byteSize;
		std::unique_ptr<char[]> buffer = std::make_unique<char[]>(byteLength + 1); // NOLINT
		std::memset(buffer.get(), 0, byteLength + 1);
		int bufferSize = byteLength;

		if (ec_SDOread(nameObject.getSlaveID(), nameObject.getIndex(), nameObject.getSubIndex(),
		        (boolean) false, &bufferSize, buffer.get(), EC_TIMEOUTRXM)
		    != 1)
		{
			return ec_slave[nameObject.getSlaveID()].name; // NOLINT
		}
		return std::string(buffer.get()); // NOLINT
	}
} // namespace etherkitten::reader::bSInformant
