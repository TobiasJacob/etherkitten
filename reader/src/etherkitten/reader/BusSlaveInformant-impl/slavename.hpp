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
 * \brief Defines methods for reading the names of slaves from an EtherCAT bus.
 */

#include <string>

#include <etherkitten/datatypes/dataobjects.hpp>
#include <etherkitten/datatypes/esidata.hpp>

#include "coe.hpp"

namespace etherkitten::reader::bSInformant
{
	/*!
	 * \brief Get the name of the given slave via SOEM.
	 * \param slave the slave ID (1-indexed)
	 * \return the name of the slave as SOEM tells us
	 */
	std::string getSlaveName(int slave);

	/*!
	 * \brief Get the name of the given slave by reading the CoE dictionary.
	 * \param slave the slave ID (1-indexed)
	 * \param coes the CoE dictionary
	 * \return the name of the slave as read from the CoE
	 */
	std::string getSlaveName(int slave, const CoEResult& coes);
} // namespace etherkitten::reader::bSInformant
