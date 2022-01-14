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

#include <etherkitten/config/ConfigObserver.hpp>

namespace etherkitten::config
{
	class ConfigObserverDummy : public ConfigObserver
	{
	public:
		std::optional<std::string> busId;
		bool configNotified;
		BusConfig busConfig;
		bool layoutNotified;
		BusLayout busLayout;
		bool logFolderPathNotified;
		std::filesystem::path logFolderPath;
		bool maximumMemoryNotified;
		size_t maximumMemory;

		void onBusLayoutChanged(BusLayout busLayout, std::string busId)
		{
			this->busLayout = busLayout;
			this->busId = busId;
			this->configNotified = false;
			this->layoutNotified = true;
			this->logFolderPathNotified = false;
			this->maximumMemoryNotified = false;
		}
		void onBusConfigChanged(BusConfig busConfig, std::optional<std::string> busId)
		{
			this->busConfig = busConfig;
			this->busId = busId;
			this->configNotified = true;
			this->layoutNotified = false;
			this->logFolderPathNotified = false;
			this->maximumMemoryNotified = false;
		}
		void onLogPathChanged(std::filesystem::path newLogFolderPath)
		{
			this->logFolderPath = newLogFolderPath;
			this->configNotified = false;
			this->layoutNotified = false;
			this->logFolderPathNotified = true;
			this->maximumMemoryNotified = false;
		}
		void onMaximumMemoryChanged(size_t newMaximumMemory)
		{
			this->maximumMemory = newMaximumMemory;
			this->configNotified = false;
			this->layoutNotified = false;
			this->logFolderPathNotified = false;
			this->maximumMemoryNotified = true;
		}
	};
} // namespace etherkitten::config
