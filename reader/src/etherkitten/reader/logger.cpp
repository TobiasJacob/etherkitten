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

#include "logger.hpp"

#include <fstream>
#include <mutex>
#include <thread>
#include <unordered_set>

#include "log/CoEUpdate.hpp"
#include "log/Serialized.hpp"
#include "log/Serializer.hpp"
#include "log/coe.hpp"
#include "log/coedata.hpp"
#include "log/error.hpp"
#include "log/esi.hpp"
#include "log/neighbors.hpp"
#include "log/pdo.hpp"
#include "log/pdodetails.hpp"
#include "log/processdata.hpp"
#include "log/slave.hpp"
#include "log/slavedetails.hpp"

namespace etherkitten::reader
{
	Logger::Logger(SlaveInformant& slaveInformant, Reader& reader,
	    std::shared_ptr<datatypes::ErrorIterator> errorIterator, std::filesystem::path&& logFile)
	    : Logger(slaveInformant, reader, errorIterator, std::move(logFile), [](int, std::string) {})
	{
	}

	Logger::Logger(SlaveInformant& slaveInformant, Reader& reader,
	    std::shared_ptr<datatypes::ErrorIterator> errorIterator, std::filesystem::path&& logFile,
	    std::function<void(int, std::string)> progressFunction)
	    : ostrm(logFile, std::ios::binary)
	    , slaveInformant(slaveInformant)
	    , reader(reader)
	    , errorWrapper(errorIterator)
	    , progressFunction(progressFunction)
	{
	}

	Logger::~Logger() { stopLog(); }

	void Logger::fetchDataViews()
	{
		// Delete all register wrappers from previous logs
		registerWrappers.clear();

		// Also clear NewestValueViews from previous logs
		registerNewestValues.clear();

		// Fetch data views for new log
		for (size_t i = 1; i <= slaveInformant.getSlaveCount(); ++i)
		{
			fetchDataViews(slaveInformant.getSlaveInfo(i));
		}
	}

	void Logger::fetchDataViews(const datatypes::SlaveInfo& slaveInfo)
	{
		std::unordered_set<uint16_t> regSet;
		for (auto& reg : slaveInfo.getRegisters())
		{
			uint16_t regAddress = static_cast<uint16_t>(reg.getRegister());
			if (regSet.count(regAddress) > 0)
				continue;
			regSet.emplace(regAddress);
			std::shared_ptr<datatypes::AbstractDataView> dataView(
			    reader.getRegisterRawDataView(reg.getSlaveID(), regAddress, { startTime, 0s }));
			registerWrappers.push_back(
			    RegisterDataViewWrapper(regAddress, reg.getSlaveID(), dataView));

			// For progress calculation
			registerNewestValues.emplace_back(reader.getNewest(reg));
		}
	}

	void Logger::startLog(datatypes::TimeStamp time)
	{
		if (state == LoggerState::NO_LOG)
		{
			startTime = time;
			fetchDataViews();
			ioMapWrapper = std::make_unique<IOMapDataViewWrapper>(reader.getIOMapView(startTime));
			thread = std::thread(&Logger::writeLog, this);
		}
		else
		{
			throw std::runtime_error("cannot start logger because it is already running");
		}
	}

	void Logger::stopLog()
	{
		if (state != LoggerState::NO_LOG && state != LoggerState::STOPPING)
		{
			// Wait for logger thread to finish
			setState(LoggerState::STOPPING);
			thread.join();

			// Delete all register wrappers from previous log
			registerWrappers.clear();
		}
	}

	void Logger::write64(uint64_t data)
	{
		data = flipBytesIfBigEndianHost(data);
		ostrm.write(reinterpret_cast<char*>(&data), sizeof data);
	}

	void Logger::writeSerialized(Serialized& ser) { ostrm.write(ser.data, ser.length); }
	void Logger::writeSerialized(Serialized&& ser) { ostrm.write(ser.data, ser.length); }

	void Logger::writeSlaveInfo(datatypes::SlaveInfo& slaveInfo)
	{
		std::vector<PDOBlock> pdoBlocks;
		for (auto& pdo : slaveInfo.getPDOs())
		{
			datatypes::PDOInfo info = reader.getAbsolutePDOInfo(pdo);
			pdoBlocks.emplace_back(pdo.getIndex(), info.bitOffset,
			    static_cast<uint16_t>(pdo.getType()), pdo.getName());
		}
		std::vector<CoEEntryBlock> coeEntryBlocks;
		for (auto& coeEntry : slaveInfo.getCoEs())
		{
			std::vector<CoEBlock> coeBlocks;
			for (auto& coe : coeEntry.getObjects())
			{
				coeBlocks.push_back(CoEBlock(coe.getIndex(), coe.getSubIndex(),
				    static_cast<uint16_t>(coe.getType()), coe.getName()));
			}
			coeEntryBlocks.emplace_back(static_cast<uint16_t>(coeEntry.getIndex()),
			    static_cast<uint8_t>(coeEntry.getObjectCode()), std::move(coeEntry.getName()),
			    std::move(coeBlocks));
		}
		ESIBlock esiBlock(slaveInfo.getESIBinary());
		NeighborsBlock nBlock(slaveInfo.getNeighbors());
		uint16_t slaveId = slaveInfo.getID();
		std::string name = slaveInfo.getName();
		SlaveBlock slaveBlock(slaveId, name, pdoBlocks, coeEntryBlocks, esiBlock, nBlock);
		auto& serializer = slaveBlock.getSerializer();
		Serialized ser(serializer.serialize(slaveBlock));
		writeSerialized(ser);

		// Write PDO details offset
		uint64_t pos = ostrm.tellp();
		ostrm.seekp(8);
		write64(pos);
		ostrm.seekp(pos);
	}

	void Logger::writePDODetails(datatypes::SlaveInfo& slaveInfo)
	{
		std::vector<PDODetailsBlock> blocks;
		for (auto& pdo : slaveInfo.getPDOs())
		{
			auto pdoInfo = reader.getAbsolutePDOInfo(pdo);
			blocks.emplace_back(pdo.getIndex(), pdoInfo.bitOffset, pdoInfo.bitLength,
			    static_cast<uint16_t>(pdo.getType()));
		}
		SlaveDetailsBlock sB(slaveInfo.getID(), blocks);
		auto& serializer = sB.getSerializer();
		Serialized ser(serializer.serialize(sB));
		writeSerialized(ser);

		// Write data offset
		uint64_t pos = ostrm.tellp();
		ostrm.seekp(16);
		write64(pos);
		ostrm.seekp(pos);
	}

	void Logger::writeLog()
	{
		progressFunction(0, "Writing slave info");
		setState(LoggerState::WRITE_SLAVE_INFO);
		// Write version
		uint64_t version = 1;
		write64(version);

		// Write offset placeholder
		uint64_t zero = 0;
		write64(zero);
		write64(zero);

		// Write ioMap size
		write64(slaveInformant.getIOMapSize());

		// Write start timestamp
		write64(datatypes::timeStampToInt(reader.getStartTime()));

		// Write SlaveInfo (note that the master is not written to the log since we get no
		// information)
		for (size_t i = 1; i <= slaveInformant.getSlaveCount(); ++i)
		{
			auto slaveInfo = slaveInformant.getSlaveInfo(i);
			writeSlaveInfo(slaveInfo);
		}

		progressFunction(0, "Writing pdo description");
		setState(LoggerState::WRITE_PDO_DET);

		// Write PDO serialization details
		for (size_t i = 1; i <= slaveInformant.getSlaveCount(); ++i)
		{
			auto slaveInfo = slaveInformant.getSlaveInfo(i);
			writePDODetails(slaveInfo);
		}

		reportProgress();
		setState(LoggerState::WRITE_DATA);

		while (state != LoggerState::STOPPING)
		{
			if (progressCounter >= progressCounterMax)
			{
				progressCounter = 0;
				reportProgress();
				reachedEndOfPD = false;
			}

			if (!writeNextBlock())
			{
				std::this_thread::sleep_for(1ms);
			}
		}

		setState(LoggerState::NO_LOG);
	}

	void Logger::setState(LoggerState state)
	{
		std::lock_guard<std::mutex> lock(mutex);
		if (this->state != LoggerState::STOPPING)
			this->state = state;
		else if (state == LoggerState::NO_LOG)
			this->state = state;
	}

	bool Logger::writeNextBlock()
	{
		bool blockWritten = false;
		progressCounter++;

		// CoE updates have absolute priority
		{
			std::scoped_lock<std::mutex> lock(coeMutex);
			if (!coeQueue.empty())
			{
				CoEUpdate& update = coeQueue.front();
				Serialized ser(update.get());
				coeQueue.pop();
				writeSerialized(ser);
				return true;
			}
		}

		// balance registers and process data
		readerCounter++;
		if (readerCounter >= 6)
			readerCounter = 0;
		if (readerCounter < 3)
		{
			// write register
			for (auto& wrp : registerWrappers)
			{
				if (wrp.hasNext())
				{
					wrp.next();
					writeSerialized(wrp.get());
					blockWritten = true;
				}
			}
		}

		if (errorWrapper.hasNext())
		{
			++errorWrapper;
			datatypes::TimeStamp time = (*errorWrapper).getTime();
			datatypes::ErrorMessage message = (*errorWrapper).getValue();
			ErrorBlock errorBlock(static_cast<uint8_t>(message.getSeverity()), message.getMessage(),
			    datatypes::timeStampToInt(time), message.getAssociatedSlaves().first,
			    message.getAssociatedSlaves().second);
			writeSerialized(errorBlock.getSerializer().serialize(errorBlock));
			blockWritten = true;
		}

		// write process data
		return writeProcessDataBlock() || blockWritten;
	}

	bool Logger::writeProcessDataBlock()
	{
		if (!ioMapWrapper->hasNext())
		{
			reachedEndOfPD = true;
			return false;
		}

		ioMapWrapper->next();
		auto ioMap = ioMapWrapper->get();
		Serialized ser(ioMap->ioMapSize);
		memcpy(ser.data, ioMap->ioMap, ioMap->ioMapSize);
		ProcessDataBlock block{ datatypes::timeStampToInt(ioMapWrapper->getTime()),
			std::move(ser) };
		writeSerialized(block.getSerializer().serialize(block));
		return true;
	}

	void Logger::updateCoE(const datatypes::CoEObject& object,
	    std::shared_ptr<datatypes::AbstractDataPoint>&& dataPoint)
	{
		std::scoped_lock<std::mutex> lock(coeMutex);
		coeQueue.push(CoEUpdate(object, std::move(dataPoint)));
	}

	void Logger::reportProgress()
	{
		double progress = 0;
		// count of all different register blocks that should be in the log
		unsigned long long registerCount = 0;

		for (size_t i = 0; i < registerWrappers.size(); i++)
		{
			auto& regWrap = registerWrappers.at(i);
			if (!regWrap.isEmpty())
			{
				registerCount++;
				auto diff = regWrap.getTime() - startTime;
				auto diff2 = (**registerNewestValues.at(i)).getTime() - startTime;
				progress
				    += (static_cast<double>(diff.count()) / static_cast<double>(diff2.count()));
			}
		}

		if (registerCount != 0)
			progress /= static_cast<double>(registerCount);

		if (!reachedEndOfPD)
			progress *= .95;

		std::stringstream s;
		s << "Writing data, register progress: " << static_cast<int>(progress * 100) << "%";
		if (reachedEndOfPD)
			s << ", all processdata written";
		progressFunction(progress * 100, s.str());
	}

} // namespace etherkitten::reader
