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

#include "LogReader.hpp"

#include <any>
#include <fstream>

#include <etherkitten/datatypes/datapoints.hpp>
#include <etherkitten/datatypes/ethercatdatatypes.hpp>
#include <etherkitten/datatypes/time.hpp>

#include "DatatypesSerializer.hpp"
#include "log/Serialized.hpp"
#include "log/coedata.hpp"
#include "log/error.hpp"
#include "log/processdata.hpp"
#include "log/registerdata.hpp"
#include "log/slavedetails.hpp"

namespace etherkitten::reader
{
	LogReader::LogReader(
	    std::filesystem::path logFile, LogSlaveInformant& slaveInformant, LogCache& logCache)
	    : LogReader(logFile, slaveInformant, logCache, [](int, std::string) {})
	{
	}

	LogReader::LogReader(std::filesystem::path logFile, LogSlaveInformant& slaveInformant,
	    LogCache& logCache, std::function<void(int, std::string)> progressFunction)
	    : SearchListReader(getSlaveConfiguredAddresses(slaveInformant.getSlaveCount()),
	          slaveInformant.getIOMapSize(), slaveInformant.getStartTime())
	    , logFile(logFile)
	    , logSlaveInformant(slaveInformant)
	    , logCache(logCache)
	    , progressFunction(progressFunction)
	    , readerThread(new std::thread(&LogReader::initReaderThread, this))
	{
	}

	LogReader::~LogReader()
	{
		shouldHalt = true;
		// and wait for it to finish
		readerThread->join();
	}

	datatypes::PDOInfo LogReader::getAbsolutePDOInfo(const datatypes::PDO& pdo)
	{
		return logSlaveInformant.busInfo.pdoOffsets.at(pdo);
	}

	void LogReader::changeRegisterSettings(
	    const std::unordered_map<datatypes::RegisterEnum, bool>& toRead)
	{
		(void)toRead;
		// This can be ignored.
	}

	void LogReader::toggleBusSafeOp()
	{
		// Does not do anything for LogReader
	}

	datatypes::BusMode LogReader::getBusMode() { return datatypes::BusMode::READ_ONLY; }

	void LogReader::messageHalt() { shouldHalt = true; }

	void LogReader::initReaderThread()
	{
		try
		{
			readLog();
		}
		catch (const std::exception& e)
		{
			logCache.postError(datatypes::ErrorMessage(
			    "Log cannot be read. Thread stopped.", datatypes::ErrorSeverity::FATAL));
		}
	}

	void LogReader::readLog()
	{
		ParsingContext parsingContext{ logSlaveInformant, logSlaveInformant.busInfo };
		std::ifstream fin(logFile, std::ios::in | std::ios::binary);
		uint64_t fileSize = std::filesystem::file_size(logFile);
		uint64_t pdoDescOffset;
		uint64_t dataOffset;
		{
			/* Log header
			 * *---------*------------------------*-------------*------------*
			 * | Version | PDO description offset | data offset |            |
			 * |---------|------------------------|-------------|------------|
			 * | 0 - 7   | 8 - 16                 | 16 - 23     | 24 - 31    |
			 * *---------*------------------------*-------------*------------*
			 */
			Serialized tmp(24);
			fin.read(tmp.data, 24);
			pdoDescOffset = tmp.read<uint64_t>(8);
			dataOffset = tmp.read<uint64_t>(16);
		}

		fin.seekg(dataOffset);
		uint64_t off = dataOffset;

		/*
		 * After the PDO descriptions the log file contains the data blocks with
		 * the processdata, register data and CoE data
		 */
		while (fin.good() && !shouldHalt)
		{
			freeMemoryIfNecessary();
			// Buffer for 4 byte identification
			Serialized tmp(4);
			fin.read(tmp.data, 4);

			// If reading goes wrong or we are at the end finish
			if (!fin.good())
				break;

			reportProgress(static_cast<uint64_t>((static_cast<double>(off) / fileSize) * 100.0),
			    "Reading logfile");

			uint32_t ident = tmp.read<uint32_t>(0);
			if (((ident & 0xFF000000) >> 24) == 0x80)
			{
				// Parse IOMap
				uint64_t ioMapSize = parsingContext.busInfo.ioMapUsedSize;
				Serialized ser(12 + ioMapSize);
				memcpy(ser.data, tmp.data, 4);
				fin.read(ser.data + 4, ser.length - 4);
				ProcessDataBlock block
				    = ProcessDataBlock::serializer.parseSerialized(ser, parsingContext);

				std::unique_ptr<IOMap> ptr(new (ioMapSize) IOMap{ ioMapSize, {} });
				std::memcpy(ptr->ioMap, block.data.data, ioMapSize);
				datatypes::TimeStamp time = datatypes::intToTimeStamp(block.timestamp);
				insertIOMap(std::move(ptr), std::move(time));
				off += ser.length;
			}
			else if (((ident & 0xFF000000) >> 24) == 0x90)
			{
				// Parse CoE data
				uint16_t slave = (ident & 0xFFFF);
				Serialized tmp2(16);
				fin.read(tmp2.data, 16);
				uint64_t length = tmp2.read<uint64_t>(8);
				Serialized ser(length);
				memcpy(ser.data, tmp.data, 4);
				memcpy(ser.data + 4, tmp2.data, 16);
				fin.read(ser.data + 20, length - 20);
				CoEDataBlock dataBlock
				    = CoEDataBlock::serializer.parseSerialized(ser, parsingContext);
				try
				{
					insertCoE(findCoEObject(slave, dataBlock.index, dataBlock.subIndex),
					    dataBlock.timestamp, dataBlock.data);
				}
				catch (const std::runtime_error& e)
				{
					logCache.postError(datatypes::ErrorMessage{
					    "Reading CoEObject of unknown CoEObject from log, " + std::string(e.what()),
					    datatypes::ErrorSeverity::LOW });
				}

				off += length;
			}
			else if (((ident & 0xFF000000) >> 24) == 0xA0)
			{
				// Parse error message
				Serialized tmp2(16);
				fin.read(tmp2.data, 16);
				uint64_t length = tmp2.read<uint64_t>(8);
				Serialized ser(length);
				memcpy(ser.data, tmp.data, 4);
				memcpy(ser.data + 4, tmp2.data, 16);
				fin.read(ser.data + 20, length - 20);
				ErrorBlock errorBlock = ErrorBlock::serializer.parseSerialized(ser, parsingContext);
				logCache.postError(datatypes::ErrorMessage{ std::move(errorBlock.message),
				                       { errorBlock.getSlave1(), errorBlock.getSlave2() },
				                       static_cast<datatypes::ErrorSeverity>(errorBlock.severity) },
				    datatypes::intToTimeStamp(errorBlock.time));
				off += length;
			}
			else
			{
				// Parse register data
				uint16_t reg = ((ident & 0xFFFF0000) >> 16);
				uint16_t slave = (ident & 0xFFFF);
				datatypes::RegisterEnum regId = static_cast<datatypes::RegisterEnum>(reg);
				uint64_t length = datatypes::getRegisterByteLength(regId);

				Serialized ser(12 + length);
				memcpy(ser.data, tmp.data, 4);
				fin.read(ser.data + 4, 12 + length - 4);
				RegisterDataBlock dataBlock
				    = RegisterDataBlock::serializer.parseSerialized(ser, parsingContext);
				try
				{
					insertRegister(
					    findRegisterObject(slave, regId), dataBlock.timestamp, dataBlock.data);
				}
				catch (const std::runtime_error& e)
				{
					logCache.postError(datatypes::ErrorMessage{
					    "Reading Register value of unknown Register or slave from log, "
					        + std::string(e.what()),
					    datatypes::ErrorSeverity::LOW });
				}
				off += length + 12;
			}
		}

		progressFunction(100, "Finished reading logfile");
	}

	void LogReader::reportProgress(unsigned int progress, std::string text)
	{
		if (progCnt++ > 1000)
		{
			progCnt = 0;
			progressFunction(progress, text);
		}
	}

	void LogReader::insertRegister(
	    const datatypes::Register& reg, uint64_t timestamp, uint64_t data)
	{
		datatypes::TimeStamp time = datatypes::intToTimeStamp(timestamp);
		SearchListReader::insertRegister(
		    reg.getRegister(), reinterpret_cast<uint8_t*>(&data), reg.getSlaveID(), time);
	}

	/*!
	 * \brief Converts an EtherCATDataType from an std::any to an AbstractDataPoint.
	 *
	 * To be used with the dataTypeMaps.
	 * \tparam E the type inside the std::any
	 */
	template<datatypes::EtherCATDataTypeEnum E, typename...>
	class AnyToPoint
	{
	public:
		using product_t = std::function<std::unique_ptr<datatypes::AbstractDataPoint>(
		    std::any, datatypes::TimeStamp)>;

		static product_t eval()
		{
			return [](std::any data,
			           datatypes::TimeStamp time) -> std::unique_ptr<datatypes::AbstractDataPoint> {
				using Type = typename datatypes::TypeMap<E>::type;
				return std::make_unique<datatypes::DataPoint<Type>>(
				    datatypes::DataPoint<Type>{ std::any_cast<Type>(data), time });
			};
		}
	};

	void LogReader::insertCoE(const datatypes::CoEObject& reg, uint64_t timestamp, std::any data)
	{
		std::unique_ptr<datatypes::AbstractDataPoint> datapoint
		    = datatypes::dataTypeMapWithStrings<AnyToPoint>.at(reg.getType())(
		        data, datatypes::intToTimeStamp(timestamp));
		logCache.setCoEValue(reg, std::move(datapoint));
	}

	const datatypes::CoEObject& LogReader::findCoEObject(
	    uint16_t slave, uint16_t index, uint8_t subindex) const
	{
		auto& entries = logSlaveInformant.getSlaveInfo(slave).getCoEs();
		for (auto& entry : entries)
		{
			for (auto& obj : entry.getObjects())
			{
				if (obj.getIndex() == index && obj.getSubIndex() == subindex)
					return obj;
			}
		}
		std::stringstream sstr;
		sstr << "No CoE object found with index " << index << " and subindex " << subindex
		     << std::endl;
		throw std::runtime_error(sstr.str());
	}

	const datatypes::Register& LogReader::findRegisterObject(
	    uint16_t slave, datatypes::RegisterEnum regId) const
	{
		auto& regs = logSlaveInformant.getSlaveInfo(slave).getRegisters();
		for (auto& reg : regs)
		{
			if (reg.getRegister() == regId)
				return reg;
		}

		std::stringstream sstr;
		sstr << "No Register object found with enumtype " << static_cast<uint16_t>(regId)
		     << " for slave " << slave << std::endl;
		throw std::runtime_error(sstr.str());
	}

	std::vector<uint16_t> LogReader::getSlaveConfiguredAddresses(uint64_t slaveCount)
	{
		std::vector<uint16_t> slaveAddresses;
		for (uint32_t i = 0; i <= slaveCount; ++i)
			slaveAddresses.push_back(i + 1);
		return slaveAddresses;
	}

} // namespace etherkitten::reader
