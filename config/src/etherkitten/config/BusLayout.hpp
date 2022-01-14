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

#include <vector>

#include "Position.hpp"

namespace etherkitten::config
{
	/*!
	 * \brief Represents a configuration of the slave graph.
	 *
	 * This class can be stored in a config file by ConfigIO.
	 */
	class BusLayout
	{
	public:
		/*!
		 * \brief Create an empty BusLayout with all Positions set to (0,0).
		 */
		BusLayout();

		/*!
		 * \brief Set the position of a slave on the graph.
		 * Slave #0 is the position of the master.
		 *
		 * \param slave The position of the slave in the bus as seen by the master.
		 * \param pos The position that the slave should have in the graph.
		 */
		void setPosition(unsigned int slave, Position& pos);

		/*!
		 * \brief Return the Position of a given slave.
		 * Slave #0 is the position of the master.
		 *
		 * The Position of slaves > getSlaveCount() is always (0,0).
		 * \param slave The position of the slave in the bus as seen by the master.
		 * \return The position of the slave in the graph.
		 */
		Position getPosition(unsigned int slave) const;

		/*!
		 * \brief Returns how many slaves are in this BusLayout.
		 *
		 * Slaves with position (0,0) may or may not be counted.
		 * Requesting the Position of a slave > getSlaveCount() will always return the Position
		 * (0,0). \return The number of slaves.
		 */
		unsigned int getSlaveCount() const;

	private:
		std::vector<Position> slavePositions;
	};
} // namespace etherkitten::config
