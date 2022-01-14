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
 * \brief Defines methods for reading ESI information from an EtherCAT bus.
 */

#include "impl-common.hpp"

namespace etherkitten::reader::bSInformant
{
	/*!
	 * \brief Read the ESI of the slave into a vector of bytes.
	 *
	 * This function will stop reading after the end category marker in the SII.
	 * Up to 7 more bytes may follow it depending on its alignment.
	 * A MEDIUM error will be contained in the errors iff reading the EEPROM fails completely.
	 * If only some reads fail, undefined values will be returned for those reads instead.
	 * \param slave the slave to read the ESI of
	 * \return the bytes that were read and / or the errors that occurred
	 */
	MayError<std::vector<std::byte>> readESIBinary(unsigned int slave);
} // namespace etherkitten::reader::bSInformant
