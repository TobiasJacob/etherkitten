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

#include "BusConfig.hpp"

namespace etherkitten::config
{

	BusConfig::BusConfig() {}

	BusConfig::BusConfig(BusConfig& busConfig) { visibleRegisters = busConfig.visibleRegisters; }

	BusConfig::BusConfig(const BusConfig& busConfig)
	{
		visibleRegisters = busConfig.visibleRegisters;
	}

	bool BusConfig::isRegisterVisible(uint32_t registerAddr) const
	{
		return visibleRegisters.count(registerAddr) != 0;
	}

	void BusConfig::setRegisterVisible(uint32_t registerAddr, bool visible)
	{
		if (visible)
			visibleRegisters.insert(registerAddr);
		else
			visibleRegisters.erase(registerAddr);
	}

	std::vector<uint32_t> BusConfig::getVisibleRegisters() const
	{
		std::vector<uint32_t> vec;
		for (auto reg : visibleRegisters)
			vec.push_back(reg);
		return vec;
	}

} // namespace etherkitten::config
