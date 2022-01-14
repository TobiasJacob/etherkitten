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

#include "SlaveInformantMock.hpp"
#include <etherkitten/reader/SearchListReader.hpp>

namespace etherkitten::reader
{

	class DataReaderMock : public SearchListReader
	{
	public:
		DataReaderMock(SlaveInformantMock&& slaveInformant);

		datatypes::PDOInfo getAbsolutePDOInfo(const datatypes::PDO& pdo) override;

		void changeRegisterSettings(
		    const std::unordered_map<datatypes::RegisterEnum, bool>& toRead) override;

		void toggleBusSafeOp() override;

		datatypes::BusMode getBusMode() override;

		void messageHalt() override;

		// Feed data in for tests
		void feedRegister(datatypes::Register reg, datatypes::TimeStamp time, uint64_t value);

		/*!
		 * \brief Registers a pdo object for the IOMap.
		 * That is needed before feedPDOData() is called.
		 *
		 * \param pdo the pdo that is added to the IOMap
		 */
		void appendPDOToIOMap(datatypes::PDO pdo);

		/*!
		 * \brief Insert IOMap frame into SearchList.
		 * If values for a pdo are not supplied, the pdo is set to 0 for this frame.
		 *
		 * \param values the values for each PDO object
		 */
		void feedPDOData(
		    std::vector<std::pair<datatypes::PDO, uint64_t>> values, datatypes::TimeStamp time);

		void setSlaveInfo(size_t i, datatypes::SlaveInfo slaveInfo);

		SlaveInformantMock slaveInformant;

	private:
		std::vector<uint16_t> generateSlaveConfiguredAddresses(uint64_t slaveCount);
		std::pair<uint16_t, uint16_t> getBounds(datatypes::PDO& pdo);

		std::vector<std::tuple<datatypes::PDO, uint16_t /*startBit*/, uint16_t /*length*/>>
		    pdoMapping;
		uint16_t ioMapSize = 0;
	};
} // namespace etherkitten::reader
