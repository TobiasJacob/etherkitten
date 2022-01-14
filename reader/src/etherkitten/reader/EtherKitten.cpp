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

#include "EtherKitten.hpp"
#include "etherkitten/datatypes/errors.hpp"

#include <cmath>
#include <memory>
#include <stdexcept>
#include <vector>

namespace etherkitten::reader
{
	std::unique_ptr<datatypes::AbstractNewestValueView> EtherKitten::getNewest(
	    const datatypes::PDO& pdo)
	{
		if (!reader)
		{
			throw std::logic_error("There is no reader available to get a NewestValueView from");
		}
		return reader->getNewest(pdo);
	}

	std::unique_ptr<datatypes::AbstractNewestValueView> EtherKitten::getNewest(
	    const datatypes::Register& reg)
	{
		if (!reader)
		{
			throw std::logic_error("There is no reader available to get a NewestValueView from");
		}
		return reader->getNewest(reg);
	}

	std::unique_ptr<datatypes::AbstractNewestValueView> EtherKitten::getNewest(
	    const datatypes::CoEObject& object)
	{
		if (messageProxy)
		{
			return messageProxy->getNewest(object);
		}
		else if (logCache)
		{
			return logCache->getNewest(object);
		}
		throw std::logic_error("There is no proxy available to get a NewestValueView from");
	}

	std::unique_ptr<datatypes::AbstractNewestValueView> EtherKitten::getNewest(
	    const datatypes::ErrorStatistic& errorStatistic)
	{
		if (!errorStatistician)
		{
			throw std::logic_error(
			    "There is no error statistician available to get a NewestValueView from");
		}
		return errorStatistician->getNewest(errorStatistic);
	}

	std::shared_ptr<datatypes::AbstractDataView> EtherKitten::getView(
	    const datatypes::PDO& pdo, datatypes::TimeSeries time)
	{
		if (!reader)
		{
			throw std::logic_error("There is no reader available to get a DataView from");
		}
		return reader->getView(pdo, time);
	}

	std::shared_ptr<datatypes::AbstractDataView> EtherKitten::getView(
	    const datatypes::Register& reg, datatypes::TimeSeries time)
	{
		if (!reader)
		{
			throw std::logic_error("There is no reader available to get a DataView from");
		}
		return reader->getView(reg, time);
	}

	std::shared_ptr<datatypes::AbstractDataView> EtherKitten::getView(
	    const datatypes::ErrorStatistic& errorStatistic, datatypes::TimeSeries time)
	{
		if (!errorStatistician)
		{
			throw std::logic_error(
			    "There is no error statistician available to get a DataView from");
		}
		return errorStatistician->getView(errorStatistic, time);
	}

	double EtherKitten::getPDOFrequency()
	{
		if (!reader)
		{
			return 0;
		}
		return reader->getPDOFrequency();
	}

	double EtherKitten::getRegisterFrequency()
	{
		if (!reader)
		{
			return 0;
		}
		return reader->getRegisterFrequency();
	}

	std::shared_ptr<datatypes::ErrorIterator> EtherKitten::getErrors()
	{
		if (messageProxy)
		{
			return messageProxy->getErrors();
		}
		else if (logCache)
		{
			return logCache->getErrors();
		}
		throw std::logic_error("There is no proxy available to get errors from");
	}

	void EtherKitten::setPDOValue(
	    const datatypes::PDO& pdo, std::unique_ptr<datatypes::AbstractDataPoint> value)
	{
		if (!messageProxy)
		{
			throw std::logic_error("There is no proxy available to send PDO values to");
		}
		messageProxy->setPDOValue(pdo, std::move(value));
	}

	void EtherKitten::readCoEObject(const datatypes::CoEObject& object)
	{
		std::shared_ptr<datatypes::AbstractDataPoint> readCoE
		    = datatypes::dataTypeMapWithStrings<AbstractDataPointCreator>.at(object.getType())();
		updateCoEObject(object, std::move(readCoE), true);
	}

	void EtherKitten::writeCoEObject(
	    const datatypes::CoEObject& object, std::unique_ptr<datatypes::AbstractDataPoint> value)
	{
		std::shared_ptr<datatypes::AbstractDataPoint> writtenCoE(value.release());
		updateCoEObject(object, std::move(writtenCoE), false);
	}

	void EtherKitten::resetErrorRegisters(unsigned int slave)
	{
		if (!messageProxy)
		{
			throw std::logic_error(
			    "There is no proxy available to send register reset requests to");
		}
		messageProxy->resetErrorRegisters(slave);
	}

	unsigned int EtherKitten::getSlaveCount()
	{
		if (!slaveInfo)
		{
			throw std::logic_error(
			    "There is no slave informant available to get a slave count from");
		}
		return slaveInfo->getSlaveCount();
	}

	const datatypes::SlaveInfo& EtherKitten::getSlaveInfo(unsigned int slaveIndex)
	{
		if (!slaveInfo)
		{
			throw std::logic_error(
			    "There is no slave informant available to get slave information from");
		}
		return slaveInfo->getSlaveInfo(slaveIndex);
	}

	datatypes::ErrorStatistic& EtherKitten::getErrorStatistic(
	    datatypes::ErrorStatisticType type, unsigned int slaveId)
	{
		if (!errorStatistician)
		{
			throw std::logic_error(
			    "There is no error statistician available to get an error statistic from");
		}
		return errorStatistician->getErrorStatistic(type, slaveId);
	}

	void EtherKitten::startLogging(std::filesystem::path logFile, datatypes::TimeStamp time)
	{
		startLogging(logFile, time, [](int, std::string) {});
	}

	void EtherKitten::startLogging(std::filesystem::path logFile, datatypes::TimeStamp time,
	    std::function<void(int, std::string)> progressFunction)
	{
		if (!reader || !slaveInfo)
		{
			throw std::logic_error(
			    "There is no reader or slave informant available to start a logger with");
		}
		logger = std::make_unique<Logger>(*slaveInfo, *reader, getErrors(), std::move(logFile), std::move(progressFunction));
		logger->startLog(time);
	}

	void EtherKitten::stopLogging()
	{
		if (!logger)
		{
			throw std::logic_error("There is no logger available to stop");
		}
		logger->stopLog();
	}

	void EtherKitten::clearMembers()
	{
		errorStatistician.reset();
		logger.reset();
		reader.reset();
		messageProxy.reset();
		slaveInfo.reset();
		logCache.reset();
	}

	void EtherKitten::connectBus(
	    std::string interface, std::unordered_map<datatypes::RegisterEnum, bool>& toRead)
	{
		connectBus(interface, toRead, [](int, std::string) {});
	}

	void EtherKitten::connectBus(std::string interface,
	    std::unordered_map<datatypes::RegisterEnum, bool>& toRead,
	    std::function<void(int, std::string)> progressFunction)
	{
		clearMembers();
		slaveInfo = std::make_unique<BusSlaveInformant>(interface, std::move(progressFunction));
		connectCommonBusComponents(toRead, slaveInfo->getInitializationErrors());
	}

	void EtherKitten::connectBus(
	    int socket, std::unordered_map<datatypes::RegisterEnum, bool>& toRead)
	{
		connectBus(socket, toRead, [](int, std::string) {});
	}

	void EtherKitten::connectBus(int socket,
	    std::unordered_map<datatypes::RegisterEnum, bool>& toRead,
	    std::function<void(int, std::string)> progressFunction)
	{
		clearMembers();
		slaveInfo = std::make_unique<BusSlaveInformant>(socket, std::move(progressFunction));
		connectCommonBusComponents(toRead, slaveInfo->getInitializationErrors());
	}

	void EtherKitten::connectCommonBusComponents(
	    std::unordered_map<datatypes::RegisterEnum, bool>& toRead,
	    std::vector<datatypes::ErrorMessage> errors)
	{
		std::unique_ptr<BusQueues> queues = std::make_unique<BusQueues>();
		for (datatypes::ErrorMessage& error : errors)
		{
			queues->postError(std::move(error));
		}
		reader = std::make_unique<BusReader>(dynamic_cast<BusSlaveInformant&>(*slaveInfo),
		    dynamic_cast<BusQueues&>(*queues), toRead);
		messageProxy = std::make_unique<QueueCacheProxy>(std::move(queues));
		errorStatistician = std::make_unique<ErrorStatistician>(*slaveInfo, *reader);
		setMaximumMemory(maxMemorySize);
	}

#ifdef ENABLE_MOCKS
	void EtherKitten::startBusMock(std::unordered_map<datatypes::RegisterEnum, bool>& toRead)
	{
		clearMembers();
		slaveInfo = std::make_unique<MockSlaveInformant>();
		std::unique_ptr<BusQueues> queues = std::make_unique<BusQueues>();
		// move alle errors from bus initialization to queues
		for (datatypes::ErrorMessage errorMessage : slaveInfo->getInitializationErrors())
		{
			queues->postError(std::move(errorMessage));
		}
		reader = std::make_unique<MockReader>(dynamic_cast<BusQueues&>(*queues), toRead);
		messageProxy = std::make_unique<QueueCacheProxy>(std::move(queues));
		errorStatistician = std::make_unique<ErrorStatistician>(*slaveInfo, *reader);
	}
#endif

	void EtherKitten::detachBus()
	{
		if (!reader)
		{
			throw std::logic_error("There is no reader which could be detached");
		}
		reader->messageHalt();
	}

	void EtherKitten::loadLogFile(std::filesystem::path logFile)
	{
		return loadLog(logFile, [](int, std::string) {}, [](int, std::string) {});
	}

	void EtherKitten::loadLog(std::filesystem::path logFile,
	    std::function<void(int, std::string)> initializationProgressFunction,
	    std::function<void(int, std::string)> readingProgressFunction)
	{
		clearMembers();
		slaveInfo = std::make_unique<LogSlaveInformant>(
		    logFile, std::move(initializationProgressFunction));
		logCache = std::make_unique<LogCache>();
		// move alle errors from bus initialization to log cache
		for (datatypes::ErrorMessage errorMessage : slaveInfo->getInitializationErrors())
		{
			logCache->postError(std::move(errorMessage));
		}
		reader = std::make_unique<LogReader>(logFile, dynamic_cast<LogSlaveInformant&>(*slaveInfo),
		    dynamic_cast<LogCache&>(*logCache), std::move(readingProgressFunction));
		errorStatistician = std::make_unique<ErrorStatistician>(*slaveInfo, *reader);
		setMaximumMemory(maxMemorySize);
	}

	void EtherKitten::stopReadingLog()
	{
		if (!reader)
		{
			throw std::logic_error("There is no reader which could be detached");
		}
		reader->messageHalt();
	}

	void EtherKitten::changeRegisterSettings(
	    std::unordered_map<datatypes::RegisterEnum, bool>& toRead)
	{
		if (!reader)
		{
			throw std::logic_error(
			    "There is no reader available to change the register settings of");
		}
		reader->changeRegisterSettings(toRead);
	}

	void EtherKitten::toggleBusSafeOp()
	{
		if (!reader)
		{
			throw std::logic_error("There is no reader available to toggle the mode of");
		}
		reader->toggleBusSafeOp();
	}

	datatypes::BusMode EtherKitten::getBusMode()
	{
		if (!reader)
		{
			return datatypes::BusMode::NOT_AVAILABLE;
		}
		return reader->getBusMode();
	}

	void EtherKitten::setMaximumMemory(size_t size)
	{
		maxMemorySize = size;
		if (reader && errorStatistician)
		{
			reader->setMaximumMemory(ceil((1 - errorStatisticianMemoryProportion) * maxMemorySize));
			errorStatistician->setMaximumMemory(
			    ceil(errorStatisticianMemoryProportion * maxMemorySize));
		}
	}

	void EtherKitten::updateCoEObject(const datatypes::CoEObject& object,
	    std::shared_ptr<datatypes::AbstractDataPoint>&& value, bool readRequest)
	{
		if (!messageProxy)
		{
			throw std::logic_error("There is no proxy available to update the CoE object with");
		}
		messageProxy->updateCoEObject(object, value, readRequest);
		if (logger && value)
		{
			logger->updateCoE(object, std::move(value));
		}
	}

	datatypes::TimeStamp EtherKitten::getStartTime() const
	{
		if (reader)
		{
			return reader->getStartTime();
		}
		throw std::logic_error("No Reader is available, cannot get start time");
	}
} // namespace etherkitten::reader
