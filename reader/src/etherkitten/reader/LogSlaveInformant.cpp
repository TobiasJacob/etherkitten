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

#include "LogSlaveInformant.hpp"

#include <fstream>
#include <vector>

#include "log/Serialized.hpp"
#include "log/slave.hpp"
#include "log/slavedetails.hpp"
#include <etherkitten/datatypes/time.hpp>

namespace etherkitten::reader
{
	LogSlaveInformant::LogSlaveInformant(std::filesystem::path logFile)
	    : LogSlaveInformant(logFile, [](int, std::string) {})
	{
	}

	LogSlaveInformant::LogSlaveInformant(
	    std::filesystem::path logFile, std::function<void(int, std::string)> progressFunction)
	    : logFile(logFile)
	{
		readFile(progressFunction);
	}

	const datatypes::SlaveInfo& LogSlaveInformant::getSlaveInfo(unsigned int slaveIndex) const
	{
		if (slaveIndex < 1 || slaveIndex > slaveInfos.size())
		{
			throw std::runtime_error(
			    "Illegal slaveInfo " + std::to_string(slaveIndex) + " requested.");
		}
		if (slaveInfos.at(slaveIndex - 1).getID() != slaveIndex)
		{
			throw std::runtime_error("slaves are not in the correct order");
		}
		return slaveInfos.at(slaveIndex - 1);
	}

	unsigned int LogSlaveInformant::getSlaveCount() const { return slaveInfos.size(); }

	uint64_t LogSlaveInformant::getIOMapSize() const { return busInfo.ioMapUsedSize; }

	datatypes::TimeStamp LogSlaveInformant::getStartTime() const { return busInfo.startTime; }

	void LogSlaveInformant::readFile(std::function<void(int, std::string)> progressFunction)
	{
		progressFunction(0, "Reading log header");
		ParsingContext parsingContext(*this, busInfo);
		std::ifstream fin(logFile, std::ios::in | std::ios::binary);
		uint64_t version;
		uint64_t pdoDescOffset;
		uint64_t dataOffset;
		uint64_t off = 0;
		static const int headerSize = 40;
		{
			Serialized tmp(headerSize);
			fin.read(tmp.data, headerSize);
			off += headerSize;

			// Maybe we cannot read the logfile
			if (fin.fail())
			{
				throw SlaveInformantError{ "Failed to read log.",
					{ { "Logfile cannot be read. Maybe it is not accessible?",
							datatypes::ErrorSeverity::FATAL } } };
			}

			version = tmp.read<uint64_t>(0);
			if (version != 1)
			{
				throw SlaveInformantError{ "Failed to read log.",
					{ { "Logfile is not valid. First 64 Bits are wrong.",
					    datatypes::ErrorSeverity::FATAL } } };
			}
			pdoDescOffset = tmp.read<uint64_t>(8);
			dataOffset = tmp.read<uint64_t>(16);
			busInfo.ioMapUsedSize = tmp.read<uint64_t>(24);
			busInfo.startTime = datatypes::intToTimeStamp(tmp.read<uint64_t>(32));
		}

		// Split into Serialized objects of correct size and parse binary
		while (off + 10 < pdoDescOffset)
		{
			progressFunction(
			    100 * static_cast<double>(off) / pdoDescOffset, "Reading slave information");
			Serialized tmp(10);
			fin.read(tmp.data, 10);
			// this is the slaveId which is not required here
			tmp.read<uint16_t>(0);
			uint64_t length = tmp.read<uint64_t>(2);

			Serialized ser(length);
			memcpy(ser.data, tmp.data, 10);
			fin.read(ser.data + 10, length - 10);
			slaveInfos.push_back(SlaveBlock::serializer.parseSerialized(ser, parsingContext));

			off += length;
		}

		fin.seekg(pdoDescOffset);
		off = pdoDescOffset;
		while (off + 1 < dataOffset)
		{
			Serialized tmp(4);
			fin.read(tmp.data, 4);
			// this is the slaveId which is not used here
			tmp.read<uint16_t>(0);
			uint16_t length = tmp.read<uint16_t>(2);

			Serialized ser(length);
			memcpy(ser.data, tmp.data, 4);
			fin.read(ser.data + 4, length - 4);
			off += length;
			SlaveDetailsBlock block
			    = SlaveDetailsBlock::serializer.parseSerialized(ser, parsingContext);
			for (size_t i = 0; i < block.pdoBlocks.size(); ++i)
			{
				PDODetailsBlock& pdoBlock = block.pdoBlocks.at(i);
				try
				{
					busInfo.pdoOffsets[findPDOObject(block.slaveId, pdoBlock.index)]
					    = { .bitOffset = pdoBlock.offset, .bitLength = pdoBlock.bitLength };
				}
				catch (const std::runtime_error& e)
				{
					initializationErrors.emplace_back(
					    "Reading PDO offsets and bitlengths but the PDO "
					    "object was not defined in SlaveInfo, "
					        + std::string(e.what()),
					    datatypes::ErrorSeverity::LOW);
				}
			}
		}
		progressFunction(100, "Finished log reading");
	}

	const datatypes::PDO& LogSlaveInformant::findPDOObject(uint16_t slave, uint16_t index) const
	{
		auto& pdos = getSlaveInfo(slave).getPDOs();
		for (auto& pdo : pdos)
		{
			if (pdo.getIndex() == index)
				return pdo;
		}

		std::stringstream sstr;
		sstr << "No PDO object found with index " << index << " for slave " << slave << std::endl;
		throw std::runtime_error(sstr.str());
	}

	const std::vector<datatypes::ErrorMessage> LogSlaveInformant::getInitializationErrors()
	{
		const std::vector<datatypes::ErrorMessage> errorsTemp = initializationErrors;
		initializationErrors = std::vector<datatypes::ErrorMessage>();
		return errorsTemp;
	}
} // namespace etherkitten::reader
