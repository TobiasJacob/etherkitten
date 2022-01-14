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
 * \brief Defines the Serializer interface
 */

#include "ParsingContext.hpp"
#include "Serialized.hpp"

namespace etherkitten::reader
{
	/*!
	 * \brief Can serialize a Block and parse a Serialized object into a corresponding object.
	 * \tparam T the Block type that can be serialized
	 * \tparam R the return type of the parsing function
	 */
	template<class T, class R = T>
	class Serializer
	{
	public:
		/*!
		 * \brief Serialize the Block obj
		 * \param obj the object to serialize
		 * \return the buffer of the serialized object
		 */
		virtual Serialized serialize(const T& obj) = 0;

		/*!
		 * \brief Serialize Serialize the Block obj
		 * \param obj the object to serialize
		 * \param ser the buffer to use
		 * \exception std::runtime_error iff ser is too small
		 */
		virtual void serialize(const T& obj, Serialized& ser) = 0;

		/*!
		 * \brief Parse the buffer content. A ParsingContext object can be used to get or set
		 * metadata, that is not encoded in the return type or that is needed in the return type but
		 * not in the log file.
		 * \param data the buffer to parse
		 * \param context the ParsingContext that contains metadata
		 * \return the parsed object
		 * \exception std::runtime_error iff ser is too small
		 */
		virtual R parseSerialized(Serialized& data, ParsingContext& context) = 0;
	};
} // namespace etherkitten::reader
