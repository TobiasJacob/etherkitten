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

#include "BusLayout.hpp"

namespace etherkitten::config
{

	BusLayout::BusLayout() {}

	void BusLayout::setPosition(unsigned int slave, Position& pos)
	{
		while (slavePositions.size() < slave)
			slavePositions.push_back({ 0, 0 });

		if (slavePositions.size() == slave)
			slavePositions.push_back(pos);
		else
			slavePositions[slave] = pos;
	}

	Position BusLayout::getPosition(unsigned int slave) const
	{
		if (slave >= slavePositions.size())
			return { 0, 0 };
		return slavePositions[slave];
	}

	unsigned int BusLayout::getSlaveCount() const { return slavePositions.size(); }

} // namespace etherkitten::config
