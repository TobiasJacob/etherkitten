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

#include <etherkitten/reader/SlaveInformant.hpp>

namespace etherkitten::reader
{

	class SlaveInformantMock : public SlaveInformant
	{
	public:
		SlaveInformantMock(uint16_t slaveCount, size_t ioMapSize);
		unsigned int getSlaveCount() const override;
		const datatypes::SlaveInfo& getSlaveInfo(unsigned int slaveIndex) const override;

		uint64_t getIOMapSize() const override;

		// feed data
		void feedSlaveInfo(const uint16_t slave, datatypes::SlaveInfo&& slaveInfo);

		const std::vector<datatypes::ErrorMessage> getInitializationErrors() override;

	private:
		size_t ioMapSize;
		std::vector<datatypes::SlaveInfo> slaveInfos;
		std::vector<datatypes::ErrorMessage> initErrors;
	};

} // namespace etherkitten::reader
