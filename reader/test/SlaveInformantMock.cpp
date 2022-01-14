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

#include "SlaveInformantMock.hpp"

namespace etherkitten::reader
{
	SlaveInformantMock::SlaveInformantMock(uint16_t slaveCount, size_t ioMapSize)
	    : ioMapSize(ioMapSize)
	{
		slaveInfos.reserve(slaveCount);
		while (slaveInfos.size() < slaveCount)
		{
			slaveInfos.push_back(datatypes::SlaveInfo{ 0, "", std::vector<datatypes::PDO>(),
			    std::vector<datatypes::CoEEntry>(), datatypes::ESIData(), std::vector<std::byte>(),
			    std::array<unsigned int, 4>{ 0, 0, 0, 0 } });
		}
		initErrors.emplace_back("Testerror", datatypes::ErrorSeverity::LOW);
	}
	unsigned int SlaveInformantMock::getSlaveCount() const { return slaveInfos.size(); }
	const datatypes::SlaveInfo& SlaveInformantMock::getSlaveInfo(unsigned int slaveIndex) const
	{
		return slaveInfos.at(slaveIndex - 1);
	}

	uint64_t SlaveInformantMock::getIOMapSize() const { return ioMapSize; }

	void SlaveInformantMock::feedSlaveInfo(const uint16_t slave, datatypes::SlaveInfo&& slaveInfo)
	{
		slaveInfos.at(slave - 1) = slaveInfo;
	}

	const std::vector<datatypes::ErrorMessage> SlaveInformantMock::getInitializationErrors()
	{
		return initErrors;
	}
} // namespace etherkitten::reader
