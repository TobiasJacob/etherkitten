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

#include "DataReaderMock.hpp"

#include <etherkitten/datatypes/ethercatdatatypes.hpp>

namespace etherkitten::reader
{
	DataReaderMock::DataReaderMock(SlaveInformantMock&& slaveInformant)
	    : SearchListReader(generateSlaveConfiguredAddresses(slaveInformant.getSlaveCount()),
	          slaveInformant.getIOMapSize(), datatypes::now())
	    , slaveInformant(slaveInformant)
	{
	}

	datatypes::PDOInfo DataReaderMock::getAbsolutePDOInfo(const datatypes::PDO& pdo)
	{
		datatypes::PDOInfo info{ 0, 0 };
		for (std::tuple<datatypes::PDO, uint16_t, uint16_t>& t : pdoMapping)
		{
			if (std::get<0>(t).getSlaveID() == pdo.getSlaveID()
			    && std::get<0>(t).getIndex() == pdo.getIndex())
			{
				info.bitLength = std::get<2>(t) * 8;
				info.bitOffset = std::get<1>(t) * 8;
			}
		}
		return info;
	}

	void DataReaderMock::changeRegisterSettings(
	    const std::unordered_map<datatypes::RegisterEnum, bool>& toRead)
	{
		(void)toRead;
	}

	void DataReaderMock::toggleBusSafeOp() {}

	datatypes::BusMode DataReaderMock::getBusMode() { return datatypes::BusMode::READ_ONLY; }

	void DataReaderMock::messageHalt() {}

	std::vector<uint16_t> DataReaderMock::generateSlaveConfiguredAddresses(uint64_t slaveCount)
	{
		std::vector<uint16_t> vec;
		for (size_t i = 0; i < slaveCount; ++i)
			vec.push_back(i);
		return vec;
	}

	void DataReaderMock::feedRegister(
	    datatypes::Register reg, datatypes::TimeStamp time, uint64_t value)
	{
		insertRegister(
		    static_cast<datatypes::RegisterEnum>(static_cast<uint16_t>(reg.getRegister())),
		    reinterpret_cast<uint8_t*>(&value), reg.getSlaveID() - 1, time);
	}

	template<datatypes::EtherCATDataTypeEnum E, typename...>
	class BitLengthRetriever
	{
	public:
		using product_t = int;

		static product_t eval()
		{
			return datatypes::EtherCATDataType::bitLength<typename datatypes::TypeMap<E>::type>;
		}
	};

	void DataReaderMock::appendPDOToIOMap(datatypes::PDO pdo)
	{
		int bitLength = datatypes::dataTypeMap<BitLengthRetriever>.at(pdo.getType());
		int byteLength = bitLength / 8;
		if (byteLength * 8 < bitLength)
			byteLength++;
		pdoMapping.push_back(
		    std::tuple<datatypes::PDO, uint16_t, uint16_t>(pdo, ioMapSize, byteLength));
		ioMapSize += byteLength;
	}

	std::pair<uint16_t, uint16_t> DataReaderMock::getBounds(datatypes::PDO& pdo)
	{
		for (std::tuple<datatypes::PDO, uint16_t, uint16_t>& t : pdoMapping)
		{
			if (std::get<0>(t).getSlaveID() == pdo.getSlaveID()
			    && std::get<0>(t).getIndex() == pdo.getIndex())
				return std::pair<uint16_t, uint16_t>(std::get<1>(t), std::get<2>(t));
		}
		return std::pair<uint16_t, uint16_t>(0, 0);
	}

	void DataReaderMock::feedPDOData(
	    std::vector<std::pair<datatypes::PDO, uint64_t>> values, datatypes::TimeStamp time)
	{
		uint8_t ioMap[ioMapSize];
		for (uint16_t i = 0; i < ioMapSize; i++)
			ioMap[i] = 0;

		for (std::pair<datatypes::PDO, uint64_t>& pair : values)
		{
			std::pair<uint16_t, uint16_t> bounds = getBounds(pair.first);
			memcpy(ioMap + bounds.first, &pair.second, bounds.second);
		}
		std::unique_ptr<IOMap> ptr(new (ioMapSize) IOMap{ ioMapSize, {} });
		std::memcpy(ptr->ioMap, ioMap, ioMapSize);
		insertIOMap(std::move(ptr), std::move(time));
	}

} // namespace etherkitten::reader
