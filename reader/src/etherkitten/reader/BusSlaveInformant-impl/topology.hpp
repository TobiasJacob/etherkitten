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
 * \brief Defines methods for reading the topology of an EtherCAT bus from the bus itself.
 */

#include <array>
#include <vector>

#include "impl-common.hpp"

namespace etherkitten::reader::bSInformant
{
	/*!
	 * \brief Read the bus topology from the slave connection information
	 * provided by SOEM.
	 * \return each slave's neighbors sorted by the port on the slave they
	 * are connected to
	 */
	std::vector<std::array<unsigned int, 4>> readBusTopology();
} // namespace etherkitten::reader::bSInformant
