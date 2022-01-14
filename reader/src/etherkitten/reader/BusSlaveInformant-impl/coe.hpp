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
 * \brief Defines methods to read CoE information from an EtherCAT bus.
 */

#include "impl-common.hpp"

#include <etherkitten/datatypes/dataobjects.hpp>

namespace etherkitten::reader::bSInformant
{
	/*!
	 * \brief Read the entire CoE object dictionary of the given slave.
	 * \param slave the slave to read the dictionary of
	 * \return the CoE objects in a list and additional information in a map
	 */
	MayError<CoEResult> readObjectDictionary(unsigned int slave);
} // namespace etherkitten::reader::bSInformant
