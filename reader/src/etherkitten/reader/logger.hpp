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
 * \brief Defines the Logger, which writes logfiles to the disk.
 */

#include "DataView.hpp"
#include "ErrorStatistician.hpp"
#include "SearchListReader.hpp"
#include "SlaveInformant.hpp"
#include "log/CoEUpdate.hpp"
#include "log/DataViewWrapper.hpp"
#include "log/Serialized.hpp"
#include <etherkitten/datatypes/SlaveInfo.hpp>
#include <etherkitten/datatypes/dataviews.hpp>
#include <etherkitten/datatypes/ethercatdatatypes.hpp>
#include <etherkitten/datatypes/time.hpp>
#include <filesystem>
#include <fstream>
#include <memory>

#include <mutex>
#include <queue>
#include <thread>

namespace etherkitten::reader
{
	/*!
	 * \brief States the logger can be in
	 */
	enum LoggerState
	{
		NO_LOG,
		WRITE_SLAVE_INFO,
		WRITE_PDO_DET,
		WRITE_DATA,
		STOPPING
	};

	/*
	 * Some information about the log file:
	 * ====================================
	 *
	 * The log file is structured in different blocks. It starts with a header containing the
	 * version, pointers to block start points and the ioMap size.
	 * Then there are the SlaveBlocks that contain all the information about each slave.
	 * Then there are the SlaveDetailsBlocks that contain information about the pdo mappings.
	 * The remaining file is filled with data blocks containing the data obtained from
	 * the bus at runtime. This includes the data which can later be plotted in the GUI.
	 *
	 * For detailed information about how the blocks are structured see the files in which
	 * they are defined.
	 *
	 * The log file uses little endian byte order.
	 */

	/*!
	 * \brief A Logger that can write data, that is relevant to the user and that cannot be
	 * calculated to a logfile.
	 *
	 * This happens in another thread that is managed by this class.
	 *
	 * A SlaveInformant and Reader are used as data sources.
	 */
	class Logger
	{
	public:
		/*!
		 * \brief Create a Logger
		 *
		 * This constructor does not start the logging process itself. In order
		 * to start logging you need to call startLog().
		 *
		 * The logger writes information about the slave into the logfile.
		 * Then it writes all register data and process data with a timestamp greater
		 * than the one specified in startLog to the log.
		 * All error messages and coe updates are also included in the logfile.
		 *
		 * \param slaveInformant the SlaveInformant
		 * \param reader the Reader
		 * \param errorIterator an iterator to get errors that should be included in the log
		 * \param logFile the logFile to use
		 */
		Logger(SlaveInformant& slaveInformant, Reader& reader,
		    std::shared_ptr<datatypes::ErrorIterator> errorIterator,
		    std::filesystem::path&& logFile);

		/*!
		 * \brief Create a Logger
		 * This constructor will additionally report its progress by calling the progressFunction
		 * regularly with an integer between 0 and 100 representing its percentual progress
		 * and a string message representing its next task.
		 * \param slaveInformant the SlaveInformant
		 * \param reader the Reader
		 * \param errorIterator an iterator to get errors that should be included in the log
		 * \param logFile the logFile to use
		 * \param progressFunction a function that is called to report initialization progress
		 */
		Logger(SlaveInformant& slaveInformant, Reader& reader,
		    std::shared_ptr<datatypes::ErrorIterator> errorIterator,
		    std::filesystem::path&& logFile,
		    std::function<void(int, std::string)> progressFunction);

		~Logger();

		/*!
		 * \brief Start logging. The logger thread is started.
		 * Old data can be ignored by the logger by specifying time.
		 * Only data that is collected after this TimeStamp is written to the log.
		 * \param time the first TimeStamp of data in the log.
		 * \exception std::runtime_error iff the logger is already logging
		 */
		void startLog(datatypes::TimeStamp time);

		/*!
		 * \brief Stop logging.
		 *
		 * This interrupts the logger thread so there may be data that is collected but not written
		 * to the logfile yet.
		 *
		 * Return after the logger thread is finished.
		 */
		void stopLog();

		/*!
		 * \brief Write an update to a CoEObject to the log
		 * \param object the object which has been updated
		 * \param dataPoint the DataPoint with the new data
		 */
		void updateCoE(const datatypes::CoEObject& object,
		    std::shared_ptr<datatypes::AbstractDataPoint>&& dataPoint);

	private:
		std::ofstream ostrm;
		SlaveInformant& slaveInformant;
		datatypes::TimeStamp startTime;
		LoggerState state = NO_LOG;
		std::mutex mutex;
		std::mutex coeMutex;
		std::queue<CoEUpdate> coeQueue;
		Reader& reader;
		std::vector<RegisterDataViewWrapper> registerWrappers;
		uint64_t readerCounter = 0;
		std::unique_ptr<IOMapDataViewWrapper> ioMapWrapper;
		datatypes::FirstEmptyErrorIterator errorWrapper;

		// These are for progress calculation
		std::vector<std::unique_ptr<datatypes::AbstractNewestValueView>> registerNewestValues;
		std::function<void(int, std::string)> progressFunction;
		/*
		 * progressCounter counts up with every block. If it reaches progressCounterMax
		 * it is reset and the progress is reported.
		 */
		unsigned long progressCounter = 0;
		const unsigned long progressCounterMax = 100;
		bool reachedEndOfPD = false;

		void write64(uint64_t data);
		void writeSerialized(Serialized& ser);
		void writeSerialized(Serialized&& ser);
		void writeSlaveInfo(datatypes::SlaveInfo& slaveInfo);
		void writePDODetails(datatypes::SlaveInfo& slaveInfo);
		void writeLog();
		void setState(LoggerState state);
		bool writeNextBlock();
		bool writeProcessDataBlock();
		void fetchDataViews(const datatypes::SlaveInfo& slaveInfo);
		void fetchDataViews();
		void reportProgress();

		std::thread thread;
	};

} // namespace etherkitten::reader
