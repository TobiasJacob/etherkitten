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
 * \brief Defines the DatatypesSerializer helper methods for handling std::anys containing
 * EtherCATDataTypes.
 */

#include <any>

#include <etherkitten/datatypes/ethercatdatatypes.hpp>

#include "log/Serialized.hpp"

namespace etherkitten::reader
{
	/*!
	 * \brief Helper that contains methods to work with EtherCATDataTypeEnum and std::any.
	 */
	namespace DatatypesSerializer
	{
		/*!
		 * \brief Get the length of the data in bytes assuming it is of the given type.
		 * \param dataType the type of the data
		 * \param data the data wrapped in a std::any
		 * \exception std::out_of_range iff dataType is not a valid EtherCATDataTypeEnum value
		 * \exception std::bad_any_cast iff the type of data does not match dataType
		 * \return the length in bytes
		 */
		uint64_t getLength(datatypes::EtherCATDataTypeEnum dataType, std::any data);

		/*!
		 * \brief Serialize the data assuming it is of the given type.
		 * \param type the type of the data
		 * \param data the data wrapped in a std::any
		 * \param ser Serialized to put the binary representation of data in
		 * \exception std::out_of_range iff dataType is not a valid EtherCATDataTypeEnum value
		 * \exception std::bad_any_cast iff the type of data does not match dataType
		 * \exception std::runtime_error iff ser is not big enough
		 */
		void serialize(datatypes::EtherCATDataTypeEnum type, std::any data, Serialized& ser);

		/*!
		 * \brief Parse the Serialized object assuming it is of the specified type.
		 * \param datatype the type to interpret the data as
		 * \param ser the buffer containing the data to parse
		 * \exception std::out_of_range iff dataType is not a valid EtherCATDataTypeEnum value
		 * \exception std::runtime_error iff ser is not big enough
		 * \return the parsed data wrapped in a std::any with the corresponding EtherCATDataType
		 */
		std::any parse(datatypes::EtherCATDataTypeEnum datatype, Serialized& ser);
	}; // namespace DatatypesSerializer
} // namespace etherkitten::reader
