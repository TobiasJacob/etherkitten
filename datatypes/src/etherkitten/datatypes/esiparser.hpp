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
 * \brief Defines methods to parse ESI data from a binary representation.
 */

#include <stdexcept>

#include "esidata.hpp"
#include <stdexcept>

/*!
 * \brief Contains methods to parse ESI data from a binary representation.
 */
namespace etherkitten::datatypes::esiparser
{
	/*!
	 * \brief Parse a standard-conformant binary in SII format to an ESI structure.
	 *
	 * See ETG1000.6 for the SII specification.
	 * \param esiBinary the binary SII to parse
	 * \return the parsed ESI data structure
	 * \exception ParseException iff esiBinary is not standard-conformant
	 */
	ESIData parseESI(const std::vector<std::byte>& esiBinary);

	/*!
	 * \brief Represents an exception that occurred while parsing an ESI structure.
	 */
	class ParseException : std::runtime_error
	{
	public:
		/*!
		 * \brief Construct a new ParseException with the given message.
		 * \param msg the message to give the exception
		 */
		ParseException(std::string msg) // NOLINT
		    : std::runtime_error(msg)
		{
		}
	};
} // namespace etherkitten::datatypes::esiparser
