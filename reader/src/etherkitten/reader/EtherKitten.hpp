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
 * \brief Defines EtherKitten, a facade class for the reader library.
 */

#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>

#include <etherkitten/datatypes/SlaveInfo.hpp>
#include <etherkitten/datatypes/datapoints.hpp>
#include <etherkitten/datatypes/dataviews.hpp>
#include <etherkitten/datatypes/errors.hpp>
#include <etherkitten/datatypes/ethercatdatatypes.hpp>
#include <etherkitten/datatypes/time.hpp>

#ifdef ENABLE_MOCKS
#include <etherkitten/mocks/readermock.hpp>
#include <etherkitten/mocks/slaveinformantmock.hpp>
#endif

#include "BusQueues.hpp"
#include "BusReader.hpp"
#include "BusSlaveInformant.hpp"
#include "ErrorStatistician.hpp"
#include "LogCache.hpp"
#include "LogReader.hpp"
#include "LogSlaveInformant.hpp"
#include "MessageQueues.hpp"
#include "QueueCacheProxy.hpp"
#include "Reader.hpp"
#include "SlaveInformant.hpp"
#include "logger.hpp"
#include "queues-common.hpp"

namespace etherkitten::reader
{
	/*!
	 * \brief The facade of the reader library.
	 *
	 * This should be the only class in the reader library that is instantiated by the user.
	 * It can be instantiated once and then used forever.
	 */
	class EtherKitten
	{
	public:
		/*!
		 * \brief Get a view of the newest known value of the given PDO object
		 * \param pdo is the object to which the value relates
		 * \return a view of the newest known value of the given PDO object
		 * \exception std::logic_error iff no bus or log is currently available
		 */
		std::unique_ptr<datatypes::AbstractNewestValueView> getNewest(const datatypes::PDO& pdo);

		/*!
		 * \brief Get a view of the newest known value of the given register
		 * \param reg is the register to which the value relates
		 * \return a view of the newest known value of the given register
		 * \exception std::logic_error iff no bus or log is currently available
		 */
		std::unique_ptr<datatypes::AbstractNewestValueView> getNewest(
		    const datatypes::Register& reg);

		/*!
		 * \brief Get a view of the newest known value of the given CoE object
		 * \param object is the object to which the value relates
		 * \return a view of the newest known value of the given CoE object
		 * \exception std::logic_error iff no bus or log is currently available
		 */
		std::unique_ptr<datatypes::AbstractNewestValueView> getNewest(
		    const datatypes::CoEObject& object);

		/*!
		 * \brief Get a view of the newest known value of the given error statistic
		 * \param errorStatistic is the object to which the value relates
		 * \return a view of the newest known value of the given error statistic
		 * \exception std::logic_error iff no bus or log is currently available
		 */
		std::unique_ptr<datatypes::AbstractNewestValueView> getNewest(
		    const datatypes::ErrorStatistic& errorStatistic);

		/*!
		 * \brief Get a view of the given PDO object which follows the given time step
		 * \param pdo is the object to which the view relates
		 * \param time is the time step which the view follows
		 * \return a view of the given PDO object which follows the given time step
		 * \exception std::logic_error iff no bus or log is currently available
		 */
		std::shared_ptr<datatypes::AbstractDataView> getView(
		    const datatypes::PDO& pdo, datatypes::TimeSeries time);

		/*!
		 * \brief Get a view of the given register which follows the given time step
		 * \param register is the object to which the view relates
		 * \param time is the time step which the view follows
		 * \return a view of the given register which follows the given time step
		 * \exception std::logic_error iff no bus or log is currently available
		 */
		std::shared_ptr<datatypes::AbstractDataView> getView(
		    const datatypes::Register& reg, datatypes::TimeSeries time);

		/*!
		 * \brief Get a view of the given error statistic which follows the given time step
		 * \param errorStatistic is the object to which the view relates
		 * \param time is the time step which the view follows
		 * \return a view of the given error statistic which follows the given time step
		 * \exception std::logic_error iff no bus or log is currently available
		 */
		std::shared_ptr<datatypes::AbstractDataView> getView(
		    const datatypes::ErrorStatistic& errorStatistic, datatypes::TimeSeries time);

		/*!
		 * \brief Get the frequency (in Hz) at which the PDO objects are currently being read
		 * \return the frequency (in Hz) at which the PDO objects are currently being read
		 */
		double getPDOFrequency();

		/*!
		 * \brief Get the frequency (in Hz) at which the registers are currently being read
		 * \return the frequency (in Hz) at which the registers are currently being read
		 */
		double getRegisterFrequency();

		/*!
		 * \brief Get an iterator over the errors that have occurred in the Reader.
		 * \return an iterator over the Reader's errors
		 * \exception std::logic_error iff no bus or log is currently available
		 */
		std::shared_ptr<datatypes::ErrorIterator> getErrors();

		/*!
		 * \brief Post a request to write the given value for the given PDO.
		 * \param pdo the PDO to change the value of
		 * \param value the value to write
		 * \exception std::logic_error iff no bus or log is currently available
		 */
		void setPDOValue(
		    const datatypes::PDO& pdo, std::unique_ptr<datatypes::AbstractDataPoint> value);

		/*!
		 * \brief Post a read request for a CoEObject.
		 *
		 * This call blocks until the request has been processed.
		 * \param object the CoEObject to request the read for
		 * \exception std::logic_error iff no bus or log is currently available
		 */
		void readCoEObject(const datatypes::CoEObject& object);

		/*!
		 * \brief Post a write request for a CoEObject.
		 *
		 * This call blocks until the request has been processed.
		 * \param object the CoEObject to request the update for
		 * \param value the value to write to the CoEObject
		 * \exception std::logic_error iff no bus or log is currently available
		 */
		void writeCoEObject(const datatypes::CoEObject& object,
		    std::unique_ptr<datatypes::AbstractDataPoint> value);

		/*!
		 * \brief Post a request to reset all the error-counting registers of the slave with the
		 * given index.
		 * \param slave the index of the slave whose registers will be reset
		 * \exception std::logic_error iff no bus or log is currently available
		 */
		void resetErrorRegisters(unsigned int slave);

		/*!
		 * \brief Get the number of slaves currently available.
		 * \return the number of slaves
		 * \exception std::logic_error iff no bus or log is currently available
		 */
		unsigned int getSlaveCount();

		/*!
		 * \brief Get the SlaveInfo object for the slave with the given index.
		 *
		 * slaveIndex must be in the range `[1, getSlaveCount()]`.
		 * \param slaveIndex the index of the slave to get
		 * \return the SlaveInfo object for the slave
		 * \exception std::logic_error iff no bus or log is currently available
		 * \exception std::runtime_error iff the slaveIndex is invalid
		 */
		const datatypes::SlaveInfo& getSlaveInfo(unsigned int slaveIndex);

		/*!
		 * \brief Get the ErrorStatistic of the given type for the given slave.
		 *
		 * If type is of a non-slave-associated ErrorStatisticType, slaveId must be
		 * `std::numeric_limits<unsigned int>::max()`.
		 * \param type the type of the ErrorStatistic to get
		 * \param slaveId the ID of the slave the ErrorStatistic belongs to
		 * \return the ErrorStatistic of the given type belonging to the given slave or
		 * to no slave
		 * \exception std::out_of_range iff type is not a valid ErrorStatisticType
		 * or if type is slave-associated and slaveID does not identify a slave or
		 * if type is non-slave-associated and slaveID is not `std::numeric_limits<unsigned
		 * int>::max()`
		 * \exception std::logic_error iff no bus or log is currently available
		 */
		datatypes::ErrorStatistic& getErrorStatistic(
		    datatypes::ErrorStatisticType type, unsigned int slaveId);

		/*!
		 * \brief Start the logging of all available data after the given time in the given file.
		 * \param logFile is the path of the file where the data is written to
		 * \param time is the TimeStamp starting at which the data will be logged
		 * \exception std::logic_error iff no bus or log is currently available
		 */
		void startLogging(std::filesystem::path logFile, datatypes::TimeStamp time);

		/*!
		 * \brief starts the logging of all data after the given time in the given file
		 *
		 * This method will additionally report its initialization progress by calling the
		 * progressFunction with an integer between 0 and 100 representing its percentual progress
		 * and a string message representing its next task.
		 * \param logFile is the path of the file where the data is written to
		 * \param time is the moment since which the data is logged
		 * \param progressFunction a function that is called to report initialization
		 * progress
		 */
		void startLogging(std::filesystem::path logFile, datatypes::TimeStamp time,
		    std::function<void(int, std::string)> progressFunction);

		/*!
		 * \brief Stop logging.
		 *
		 * This interrupts the logger thread so there may be data that is collected but not written
		 * to the logfile yet.
		 * \exception std::logic_error iff no log has been started
		 */
		void stopLogging();

		/*!
		 * \brief Start the operation of the library with the bus that is connected to the given
		 * network interface.
		 * \param interface the interface the bus is connected to
		 * \param toRead the registers to read when beginning to read from the bus
		 * \exception SlaveInformantError if an unrecoverable error occurs while connecting to the
		 * bus
		 */
		void connectBus(
		    std::string interface, std::unordered_map<datatypes::RegisterEnum, bool>& toRead);

		/*!
		 * \brief Start the operation of the library with the bus that is connected to the given
		 * network interface.
		 *
		 * This method will additionally report its initialization progress by calling the
		 * progressFunction with an integer between 0 and 100 representing its percentual progress
		 * and a string message representing its next task.
		 * \param interface to which the bus is connected
		 * \param toRead the registers to read when beginning to read from the bus
		 * \param progressFunction a function that is called to report initialization progress
		 * \exception SlaveInformantError if an unrecoverable error occurs while connecting to the
		 * bus
		 */
		void connectBus(std::string interface,
		    std::unordered_map<datatypes::RegisterEnum, bool>& toRead,
		    std::function<void(int, std::string)> progressFunction);

		/*!
		 * \brief Start the operation of the library with the bus which is connected to the given
		 * network interface.
		 * \param socket to which the bus is connected
		 * \param toRead the registers to read when beginning to read from the bus
		 * \exception SlaveInformantError if an unrecoverable error occurs while connecting to the
		 * bus
		 */
		void connectBus(int socket, std::unordered_map<datatypes::RegisterEnum, bool>& toRead);

		/*!
		 * \brief Start the operation of the library with the bus which is connected to the given
		 * network interface.
		 *
		 * This method will additionally report its initialization progress by calling the
		 * progressFunction with an integer between 0 and 100 representing its percentual progress
		 * and a string message representing its next task.
		 * \param socket to which the bus is connected
		 * \param toRead the registers to read when beginning to read from the bus
		 * \param progressFunction a function that is called to report initialization progress
		 * \exception SlaveInformantError if an unrecoverable error occurs while connecting to the
		 * bus
		 */
		void connectBus(int socket, std::unordered_map<datatypes::RegisterEnum, bool>& toRead,
		    std::function<void(int, std::string)> progressFunction);

#ifdef ENABLE_MOCKS
		/*!
		 * \brief Start a mocked bus.
		 */
		void startBusMock(std::unordered_map<datatypes::RegisterEnum, bool>& toRead);
#endif
		/*!
		 * \brief Stop the operation of the library with the currently connected bus.
		 * \exception std::logic_error iff no bus is currently available
		 */
		void detachBus();

		/*!
		 * \brief Start the operation of the library by reading the given log file.
		 * \param logFile the log file to read
		 * \exception SlaveInformantError if an unrecoverable error occurs while loading the log
		 */
		void loadLogFile(std::filesystem::path logFile);

		/*!
		 * \brief Start the operation of the library with the given log
		 *
		 * This method will additionally report its initialization progress by calling the
		 * initializationProgressFunction with an integer between 0 and 100 representing its
		 * percentual progress and a string message representing its current task.
		 *
		 * It will also report its reading progress by calling the readingProgressFunction with an
		 * integer between 0 and 100 representing its percentual progress and a string message
		 * containing more detailed information on the current progress.
		 *
		 * \param logFile the logfile path
		 * \param initializationProgressFunction a function that is called to report progress
		 * \param readingProgressFunction a function that is called to report progress
		 * \exception SlaveInformantError if an unrecoverable error occurs while loading the log
		 */
		void loadLog(std::filesystem::path logFile,
		    std::function<void(int, std::string)> initializationProgressFunction,
		    std::function<void(int, std::string)> readingProgressFunction);

		/*!
		 * \brief Stop reading the currently open log.
		 * \exception std::logic_error iff no log is currently available
		 */
		void stopReadingLog();

		/*!
		 * \brief Change which registers are read and provided by the Reader.
		 * \param toRead are the new registers that should be read
		 * \exception std::logic_error iff no bus or log is currently available
		 */
		void changeRegisterSettings(std::unordered_map<datatypes::RegisterEnum, bool>& toRead);

		/*!
		 * \brief Force all the slaves of the connected bus to toggle their bus status from either
		 * OP to SafeOp or the other way around related to their current status.
		 * \exception std::logic_error iff no bus or log is currently available
		 */
		void toggleBusSafeOp();

		/*!
		 * \brief Get the current state of all slaves.
		 * \return the current state of all the slaves on the connected bus
		 */
		datatypes::BusMode getBusMode();

		/*!
		 * \brief Set the maximum amount of memory that this class may use for data storage.
		 * \param size the maximum memory size in bytes
		 */
		void setMaximumMemory(size_t size);

		/*!
		 * \brief Get a TimeStamp that is earlier than all DataPoints offered by this Reader.
		 *
		 * This TimeStamp can be used as the base time to calculate time offsets
		 * of DataPoints.
		 * \return a TimeStamp that is earlier than all DataPoints
		 */
		datatypes::TimeStamp getStartTime() const;

	private:
		void clearMembers();

		void updateCoEObject(const datatypes::CoEObject& object,
		    std::shared_ptr<datatypes::AbstractDataPoint>&& value, bool readRequest);

		void connectCommonBusComponents(std::unordered_map<datatypes::RegisterEnum, bool>& toRead,
		    std::vector<datatypes::ErrorMessage> errors);

		static constexpr float errorStatisticianMemoryProportion = 0.10;

		std::unique_ptr<SlaveInformant> slaveInfo;

		std::unique_ptr<QueueCacheProxy> messageProxy;
		std::unique_ptr<LogCache> logCache;
		std::unique_ptr<Reader> reader;
		std::unique_ptr<Logger> logger;
		std::unique_ptr<ErrorStatistician> errorStatistician;
		size_t maxMemorySize = 0;
	};
} // namespace etherkitten::reader
