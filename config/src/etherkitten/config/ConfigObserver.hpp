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

#include <optional>
#include <string>

#include "BusConfig.hpp"
#include "BusLayout.hpp"
#include <filesystem>

namespace etherkitten::config
{
	/*!
	 * \brief This interface defines a listener for config changes.
	 */
	class ConfigObserver
	{
	public:
		/*!
		 * \brief Virtual destructor for interface.
		 */
		virtual ~ConfigObserver() = 0;

		/*!
		 * \brief Gets called if the BusLayout of the bus has been read or written.
		 *
		 * \param busLayout The new BusLayout for the bus.
		 * \param busId The id / name of the bus.
		 */
		virtual void onBusLayoutChanged(BusLayout busLayout, std::string busId) = 0;

		/*!
		 * \brief Gets called if the BusConfig of the bus has  been read or written.
		 * Iff the optional has no value, busConfig is the default configuration.
		 * This is necessary since there is no busId at initialization, but a default
		 * configuration might still be needed.
		 *
		 * \param busConfig The new BusConfig for the bus.
		 * \param busId The id / name of the bus.
		 */
		virtual void onBusConfigChanged(BusConfig busConfig, std::optional<std::string> busId) = 0;

		/*!
		 * \brief Gets called if the path of the log folder has  been read or written.
		 *
		 * \param newLogFolderPath The new path of the log folder.
		 */
		virtual void onLogPathChanged(std::filesystem::path newLogFolderPath) = 0;

		/*!
		 * \brief Gets called if the maximum allowed memory has been read or written.
		 *
		 * \param newMaximumMemory The new maximum memory.
		 */
		virtual void onMaximumMemoryChanged(size_t newMaximumMemory) = 0;
	};
	inline ConfigObserver::~ConfigObserver() {}
} // namespace etherkitten::config
