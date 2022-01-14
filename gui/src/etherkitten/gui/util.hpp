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
 * \brief Defines helper methods for the GUI.
 */

#include <etherkitten/datatypes/dataobjects.hpp>

namespace etherkitten::gui
{

	/*!
	 * \brief Compare RegisterEnum values properly. This is required because the
	 * registers that are stored in the same byte have the high bits set to the
	 * bit offsets in the byte, so they can't just be sorted normally.
	 * \param a The first value.
	 * \param b The second value.
	 * \return Whether a < b or not.
	 */
	bool compareRegisters(datatypes::RegisterEnum a, datatypes::RegisterEnum b);

	/*!
	 * \brief Check if a DataObject represents a numeric type.
	 * \param obj The DataObject to check.
	 * \return Whether the DataObject represents a numeric type.
	 */
	bool dataObjectIsNumeric(const datatypes::DataObject& obj);

} // namespace etherkitten::gui
