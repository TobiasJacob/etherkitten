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
 * \brief Defines a parser for EtherCATDataTypes.
 */

#include <limits>
#include <sstream>
#include <stdexcept>

#include "dataobjects.hpp"

namespace etherkitten::datatypes
{
	/*!
	 * \brief TypeParserException is thrown if an something unexpected happens in EtherCATTypeParser
	 * and parsing cannot continue.
	 */
	class TypeParserException : public std::runtime_error
	{
	public:
		/*!
		 * \brief Create a TypeParserException carrying the specified message.
		 * \param msg the message to attach to the exception
		 */
		TypeParserException(std::string msg)
		    : runtime_error(msg)
		{
		}
	};

	template<typename T>
	/*!
	 * \brief EtherCATTypeParser provides a method to parse a string into the template type. Only
	 * types in EtherCATDataType are guaranteed to work.
	 */
	class EtherCATTypeParser
	{
	public:
		/*!
		 * \brief Parse an instance of the given type from the supplied string.
		 * This default specialization supports signed and unsigned integers and floating point
		 * numbers.
		 * Note: Negative numbers are not supported for binary and hexadecimal formats.
		 * \param str the string to parse
		 * \return the parsed instance
		 */
		static T parse(std::string& str)
		{
			if constexpr (std::is_integral<T>())
			{
				int base = 10;
				if (str.length() >= 3 && str.at(0) == '0')
				{
					if (str.at(1) == 'b' || str.at(1) == 'B')
					{ // Binary string
						// Pointer arithmetic to exclude first two characters
						str.assign(str.c_str() + 2, str.length() - 2);
						base = 2;
					}
					else if (str.at(1) == 'x' || str.at(1) == 'X')
					{ // Hexadecimal string
						// Pointer arithmetic to exclude first two characters
						str.assign(str.c_str() + 2, str.length() - 2);
						base = 16;
					}
				}
				if constexpr (std::numeric_limits<T>::is_signed)
				{
					auto value = std::stoll(str, 0, base);
					if (value < std::numeric_limits<T>::lowest()
					    || value > std::numeric_limits<T>::max())
					{
						std::ostringstream errorMsg;
						errorMsg << "has to be in range [" << std::numeric_limits<T>::lowest()
						         << ", " << std::numeric_limits<T>::max() << "]";
						throw TypeParserException(errorMsg.str());
					}
					// Now it is safe to static_cast, since range has been checked
					return static_cast<T>(value);
				}
				else
				{
					auto value = std::stoull(str, 0, base);
					if (value < std::numeric_limits<T>::lowest()
					    || value > std::numeric_limits<T>::max())
					{
						std::ostringstream errorMsg;
						errorMsg << "has to be in range [" << std::numeric_limits<T>::lowest()
						         << ", " << std::numeric_limits<T>::max() << "]";
						throw TypeParserException(errorMsg.str());
					}
					// Now it is safe to static_cast, since range has been checked
					return static_cast<T>(value);
				}
			}
			else if constexpr (std::is_floating_point<T>())
			{
				// If a binary literal is entered, it will be interpreted as IEEE 754 binary
				// representation
				if (str.length() >= 3 && str.at(0) == '0' && (str.at(1) == 'b' || str.at(1) == 'B'))
				{
					// Pointer arithmetic to exclude first two characters
					str.assign(str.c_str() + 2, str.length() - 2);
					switch (str.length())
					{
					case 32: // Single precision (float)
					{ // Since number is only 32 binary digits long, it definitely fits into
					  // uint32_t
						uint32_t parsedInt32 = static_cast<uint32_t>(std::stoull(str, 0, 2));
						return static_cast<T>(*reinterpret_cast<float*>(&parsedInt32));
					}
					case 64: // Double precision (double)
					{ // Since number is only 64 binary digits long, it definitely fits into
					  // uint64_t
						uint64_t parsedInt64 = std::stoull(str, 0, 2);
						double value = *reinterpret_cast<double*>(&parsedInt64);
						return static_cast<T>(value);
					}
					default:
						throw TypeParserException("illegal binary representation of IEEE 754 "
						                          "floating point number!");
					}
				}
				auto value = std::stold(str);
				// Now it is safe to static_cast, since range has been checked
				return static_cast<T>(value);
			}
			else
			{
				throw TypeParserException("unsupported type!");
			}
		}
	};

	template<std::size_t N>
	class EtherCATTypeParser<std::bitset<N>>
	{
	public:
		/*!
		 * \brief Parse a bitset of the given size from the supplied string.
		 * \param str the string to parse
		 * \return the parsed bitset
		 */
		static std::bitset<N> parse(std::string& str)
		{
			EtherCATDataType::UNSIGNED64 uintValue
			    = EtherCATTypeParser<EtherCATDataType::UNSIGNED64>::parse(str);
			return std::bitset<N>(uintValue);
		}
	};

	template<>
	class EtherCATTypeParser<EtherCATDataType::OCTET_STRING>
	{
	public:
		/*!
		 * \brief Parse a space-separated octet string from the supplied string.
		 * \param str the string to parse
		 * \return the parsed octet string
		 */
		static EtherCATDataType::OCTET_STRING parse(std::string& str)
		{
			std::istringstream splitStr(str);
			EtherCATDataType::OCTET_STRING result;
			std::string octetStr;
			size_t i = 0;
			try
			{
				while (splitStr >> octetStr)
				{
					EtherCATDataType::UNSIGNED8 octet
					    = EtherCATTypeParser<EtherCATDataType::UNSIGNED8>::parse(octetStr);
					result.push_back(octet);
					++i;
				}
			}
			catch (const TypeParserException& e)
			{
				throw TypeParserException("An error occurred while parsing octet #"
				    + std::to_string(i) + ": " + e.what());
			}
			return result;
		}
	};

	template<>
	class EtherCATTypeParser<std::string>
	{
	public:
		/*!
		 * \brief Parse a string. Does nothing, just for completeness.
		 * \param str the string to "parse"
		 * \return the input string
		 */
		static std::string parse(std::string& str) { return str; }
	};
} // namespace etherkitten::datatypes
