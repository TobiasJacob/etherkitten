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
/*!
 * \file
 * \brief Defines the LogReader, a Reader that reads its data from a logfile.
 */

#include <any>
#include <filesystem>
#include <map>
#include <thread>
#include <variant>

#include "LogCache.hpp"
#include "LogSlaveInformant.hpp"
#include "SearchList.hpp"
#include "SearchListReader.hpp"
#include "log/LogBusInfo.hpp"

namespace etherkitten::reader
{
	/*!
	 * \brief The LogReader is a Reader, similar to BusReader that reads from a logfile.
	 *
	 * Therefore writing data to the bus is not supported.
	 *
	 * The log reading is done in another thread, the reading progress is reported.
	 */
	class LogReader : public SearchListReader
	{
	public:
		/*!
		 * \brief Create a new LogReader that uses the given file and slaveInformant as sources.
		 * \param logFile the log file to use
		 * \param slaveInformant the slave informant to use
		 * \param logCache log cache to use
		 */
		LogReader(
		    std::filesystem::path logFile, LogSlaveInformant& slaveInformant, LogCache& logCache);

		/*!
		 * \brief Create a new LogReader that uses the given file and slaveInformant as sources.
		 * This constructor will additionally report its progress by calling the progressFunction
		 * regularly with an integer between 0 and 100 representing its percentual progress
		 * and a string message representing its next task.
		 * \param logFile the log file to use
		 * \param slaveInformant the slave informant to use
		 * \param logCache the log cache to use
		 * \param progressFunction a function that is called to report initialization progress
		 */
		LogReader(std::filesystem::path logFile, LogSlaveInformant& slaveInformant,
		    LogCache& logCache, std::function<void(int, std::string)> progressFunction);

		~LogReader();

		LogReader(LogReader&) = delete;

		LogReader(LogReader&&) = delete;

		LogReader& operator=(LogReader&) = delete;

		LogReader& operator=(LogReader&&) = delete;

		datatypes::PDOInfo getAbsolutePDOInfo(const datatypes::PDO& pdo) override;

		void changeRegisterSettings(
		    const std::unordered_map<datatypes::RegisterEnum, bool>& toRead) override;

		void toggleBusSafeOp() override;

		datatypes::BusMode getBusMode() override;

		void messageHalt() override;

	private:
		bool shouldHalt = false;
		std::filesystem::path logFile;
		LogSlaveInformant& logSlaveInformant;
		uint64_t memoryUsed = 0;
		std::function<void(int, std::string)> progressFunction;
		LogCache& logCache;

		std::unique_ptr<std::thread> readerThread;

		void initReaderThread();
		void readLog();
		void insertRegister(const datatypes::Register& reg, uint64_t timestamp, uint64_t data);
		void insertCoE(const datatypes::CoEObject& obj, uint64_t timestamp, std::any data);
		/*!
		 * \brief Get a reference to the CoEObject of given slaveid, index and subindex
		 * \param slave the slave of the CoEObject
		 * \param index the index of the CoEObject
		 * \param subindex the subindex of the CoEObject
		 * \exception std::runtime_error if no such CoEObject can be found
		 * \return the reference to the CoEObject
		 */
		const datatypes::CoEObject& findCoEObject(
		    uint16_t slave, uint16_t index, uint8_t subindex) const;

		/*!
		 * \brief Get a reference to the Register object of given slave and register id
		 * \param slave the slave
		 * \param regId the register id
		 * \exception std::runtime_error if no such CoEObject can be found
		 * \return the reference to the Register object
		 */
		const datatypes::Register& findRegisterObject(
		    uint16_t slave, datatypes::RegisterEnum regId) const;

		std::vector<uint16_t> getSlaveConfiguredAddresses(uint64_t slaveCount);

		unsigned long progCnt = 1000;
		/*!
		 * \brief Report progress every 1000 calls
		 * \param progress progress to report
		 * \param text the message associated with the progress
		 */
		void reportProgress(unsigned int progress, std::string text);
	};
} // namespace etherkitten::reader
