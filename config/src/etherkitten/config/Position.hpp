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

namespace etherkitten::config
{
	/*!
	 * \brief Represents a two-dimensional position that can be stored in a config file.
	 */
	struct Position
	{
	public:
		/*!
		 * \brief Create a Position with specified coordinates.
		 * \param x The x coordinate.
		 * \param y The y coordinate.
		 */
		Position(int x, int y);

		/*!
		 * \brief The x coordinate.
		 */
		int x;

		/*!
		 * \brief The y coordinate.
		 */
		int y;
	};
} // namespace etherkitten::config
