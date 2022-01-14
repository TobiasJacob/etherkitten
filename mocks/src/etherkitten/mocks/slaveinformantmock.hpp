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

#include "esiparsermock.hpp"
#include <algorithm>
#include <etherkitten/datatypes/SlaveInfo.hpp>
#include <etherkitten/reader/SlaveInformant.hpp>
#include <math.h>
#include <random>
#include <string>

namespace etherkitten::reader
{
	/**
	 * \brief A mocked SlaveInformant to be able to run EtherKitten in "live" mode without an actual
	 * bus.
	 */
	class MockSlaveInformant : public SlaveInformant
	{
	public:
		MockSlaveInformant();

		unsigned int getSlaveCount() const;

		const datatypes::SlaveInfo& getSlaveInfo(unsigned int slaveIndex) const;

		const std::vector<datatypes::ErrorMessage> getInitializationErrors() override;

	private:
		std::vector<datatypes::SlaveInfo> slaveInfos;
		uint64_t getIOMapSize() const;
		const datatypes::PDO generateMockedPDO(unsigned int slaveId) const;
		const datatypes::CoEEntry generateMockedCoEEntry(unsigned int slaveId) const;
		std::vector<std::array<unsigned int, 4>> generateGraph() const;
		std::string random_string() const;
		datatypes::EtherCATDataTypeEnum randomEtherCATDataTypeEnum() const;
		datatypes::CoEObject generateMockedCoEObject(unsigned int slaveId) const;
		int randomNumber(int from, int to) const;
		std::vector<datatypes::ErrorMessage> initErrors;
	};
} // namespace etherkitten::reader
