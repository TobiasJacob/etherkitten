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
 * \brief Defines the LogSlaveInfomant, which gets information about slaves from a logfile.
 */

#include <filesystem>
#include <functional>

#include <etherkitten/datatypes/time.hpp>

#include "SlaveInformant.hpp"
#include "log/LogBusInfo.hpp"

namespace etherkitten::reader
{
	/*!
	 * \brief The LogSlaveInformant class is a SlaveInformant that uses a log as data source.
	 */
	class LogSlaveInformant : public SlaveInformant
	{
	public:
		/*!
		 * \brief Create a new LogSlaveInformant that uses the given logFile as data source.
		 *
		 * When this constructor exits, all information regarding the slaves is read from the log
		 * file.
		 * \param logFile the log file that should be used as data source
		 * \exception std::runtime_error if the log file cannot be parsed
		 */
		LogSlaveInformant(std::filesystem::path logFile);

		/*!
		 * \brief Create a new LogSlaveInformant that uses the given logFile as data source.
		 *
		 * When this constructor exits, all information regarding the slaves is read from the log
		 * file.
		 * This constructor will additionally report its progress by calling the progressFunction
		 * regularly with an integer between 0 and 100 representing its percentual progress
		 * and a string message representing its next task.
		 * \param logFile the log file that should be used as data source
		 * \param progressFunction a function that is called to report initialization progress
		 * \exception std::runtime_error if the log file cannot be parsed
		 */
		LogSlaveInformant(
		    std::filesystem::path logFile, std::function<void(int, std::string)> progressFunction);

		unsigned int getSlaveCount() const override;

		const datatypes::SlaveInfo& getSlaveInfo(unsigned int slaveIndex) const override;

		uint64_t getIOMapSize() const override;

		/*!
		 * \brief Get a TimeStamp that is earlier than all DataPoints offered by this Reader.
		 *
		 * This TimeStamp can be used as the base time to calculate time offsets
		 * of DataPoints.
		 * \return a TimeStamp that is earlier than all DataPoints
		 */
		datatypes::TimeStamp getStartTime() const;

		const std::vector<datatypes::ErrorMessage> getInitializationErrors() override;

	private:
		friend class LogReader;
		/*!
		 * \brief Read all information needed by this LogSlaveInformant from the log file.
		 * \param progressFunction a function that is called to report initialization progress.
		 * \exception std::runtime_error if the log file cannot be parsed
		 */
		void readFile(std::function<void(int, std::string)> progressFunction);
		const datatypes::PDO& findPDOObject(uint16_t slave, uint16_t index) const;

		std::filesystem::path logFile;
		std::vector<datatypes::SlaveInfo> slaveInfos;
		LogBusInfo busInfo;

		std::vector<datatypes::ErrorMessage> initializationErrors;
	};
} // namespace etherkitten::reader
