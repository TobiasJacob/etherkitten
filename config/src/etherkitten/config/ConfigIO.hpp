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

#include <functional>
#include <pwd.h>
#include <string>
#include <unistd.h>
#include <vector>

#include "json.hpp"

#include "BusConfig.hpp"
#include "BusLayout.hpp"
#include "ConfigObserver.hpp"

namespace etherkitten::config
{

	/*!
	 * \brief Stores configuration objects in config files on the disks.
	 *
	 * Also reads these objects from the files. Notifies all registered observers if a configuration
	 * object is being changed or read.
	 */
	class ConfigIO
	{
	public:
		/*!
		 * \brief Create a new ConfigIO which uses the given config folder.
		 *
		 * \param configPath The path to the config folder
		 */
		ConfigIO(std::filesystem::path configPath);

		/*!
		 * \brief Write a BusConfig to the config file.
		 *
		 * \param busConfig The object that should be stored in the file.
		 * \param busId The name / id of the bus.
		 */
		void writeBusConfig(const BusConfig& busConfig, const std::string& busId);

		/*!
		 * \brief Write a BusLayout to the config file.
		 *
		 * \param busLayout The object that should be stored in the file.
		 * \param busId The name / id of the bus.
		 */
		void writeBusLayout(const BusLayout& busLayout, const std::string& busId);

		/*!
		 * \brief Store a log folder path in the config file.
		 * Triggers readLogFolderPath() after the file has been modified.
		 *
		 * \param logFolderPath The log folder path.
		 */
		void writeLogFolderPath(const std::filesystem::path logFolderPath);

		/*!
		 * \brief Store the maximum allowed memory in the config file.
		 * Triggers readMaximumMemory() after the file has been modified.
		 *
		 * \param maximumMemory The maximum allowed memory.
		 */
		void writeMaximumMemory(size_t maximumMemory);

		/*!
		 * \brief Read the default config.
		 * If the default config file does not exist create it.
		 * The read BusConfig is returned via ConfigObserver::onBusConfigChanged(BusConfig,
		 * std::optional<std::string>). The optional will not have a value.
		 */
		void readDefaultConfig();

		/*!
		 * \brief Read the BusConfig for a given bus.
		 *
		 * The read BusConfig is returned via ConfigObserver::onBusConfigChanged(BusConfig,
		 * std::optional<std::string>).
		 * The optional will contain a value equal to the busId.
		 *
		 * \param busId The name / id of the bus.
		 */
		void readBusConfig(const std::string& busId);

		/*!
		 * \brief Read the BusLayout for a given bus.
		 *
		 * The read BusLayout is returned via ConfigObserver::onBusLayoutChanged(BusLayout,
		 * std::string).
		 *
		 * \param busId The name / id of the bus.
		 */
		void readBusLayout(const std::string& busId);

		/*!
		 * \brief Read the log folder path from the config file.
		 * If the file does not exist create a file with default path.
		 * The read string is returned via ConfigObserver::onLogPathChanged(std::string).
		 */
		void readLogFolderPath();

		/*!
		 * \brief Read the maximum allowed memory from the config file.
		 * If the file does not exist create a file with default path.
		 * The read size is returned via ConfigObserver::onMaximumMemoryChanged(size_t).
		 */
		void readMaximumMemory();

		/*!
		 * \brief Retrieve the names of the busses that have a config file in the config folder.
		 * If these names are supplied as the busId in one of the other methods, no new config will
		 * be created. Instead the already available config file will be read or changed.
		 *
		 * \return The vector containing the bus names.
		 */
		const std::vector<std::string> getConfiguredBusses();

		/*!
		 * \brief Register an observer.
		 * This observer gets notified if a configuration object is read or written from or
		 * to a config file.
		 *
		 * \param observer The observer.
		 */
		void registerObserver(ConfigObserver& observer);

		/*!
		 * \brief Remove an observer.
		 * This observer does not get notified anymore by this ConfigIO object.
		 * This ConfigIO object also does not have any references to the observer after this method
		 * has been called.
		 *
		 * \param observer The observer.
		 */
		void removeObserver(const ConfigObserver& observer);

		/*!
		 * \brief Get the Home directory (or an alternative if there is not a home directory)
		 * \return The home directory
		 */
		static std::string getHomeDir()
		{
			const char* homedir;

			if ((homedir = getenv("HOME")) == NULL)
			{
				homedir = getpwuid(getuid())->pw_dir;
			}
			return std::string{ homedir };
		}

	private:
		std::vector<ConfigObserver*> activeObservers;
		const std::filesystem::path configPath;

		static inline const std::filesystem::path defaultLogFolderPath{ "~/etherkitten/logs" };

		/*!
		 * \brief Create the config directory.
		 *
		 * Create the config directory at the path specified in the constructor and create all
		 * subdirectories and files.
		 */
		void createConfigDirectory();

		nlohmann::json readJsonFromFile(const std::filesystem::path& filePath);

		void writeJsonToFile(const nlohmann::json& json, const std::filesystem::path& filePath);

		BusConfig parseBusConfig(nlohmann::json& json);

		void serializeBusConfig(const BusConfig& busConfig, nlohmann::json& json);

		BusLayout parseBusLayout(const nlohmann::json& json);

		void serializeBusLayout(const BusLayout& busLayout, nlohmann::json& json);

		BusConfig getDefaultConfig();
	};
} // namespace etherkitten::config
