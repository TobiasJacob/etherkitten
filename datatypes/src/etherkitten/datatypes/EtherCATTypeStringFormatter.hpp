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
 * \brief Defines a string formatter for EtherCATDataTypes.
 */

#include <bitset>
#include <cmath>
#include <ios>
#include <sstream>
#include <string>

#include "ethercatdatatypes.hpp"

namespace etherkitten::datatypes
{
	/*!
	 * \brief The NumberFormat enum encodes the number formats supported
	 * by the EtherCATTypeStringFormatter class.
	 */
	enum class NumberFormat
	{
		DECIMAL,
		BINARY,
		HEXADECIMAL,
	};

	/*!
	 * \brief The EtherCATTypeStringFormatter class is a string formatter for arbitrary types.
	 * \tparam T the type to format into a string
	 */
	template<typename T>
	class EtherCATTypeStringFormatter
	{
	public:
		/*!
		 * \brief Format a value of the chosen type as a string.
		 *
		 * If `std::is_arithmetic<T>()` or `std::is_same<T, std::vector<uint8_t>()` are true
		 * or if T is of the type `std::bitset<N>`, the given base is respected when
		 * converting numbers into strings. Otherwise, it is ignored and the string returned
		 * will correspond to the return value of the overloaded
		 * `operator<<(std::ostream&, T value)`.
		 * \param value the value to format into a string
		 * \param base the base to format numbers in, if applicable
		 * \return the value formatted as a string
		 */
		static std::string asString(T value, NumberFormat base)
		{
			std::stringstream stream;
			if constexpr (std::is_integral<T>())
			{
				int64_t intValue = static_cast<int64_t>(value);
				switch (base)
				{
				case NumberFormat::DECIMAL:
					stream << intValue;
					break;
				case NumberFormat::BINARY:
					stream << "0b" << std::bitset<sizeof(T) * byteSize>(intValue);
					break;
				case NumberFormat::HEXADECIMAL:
					stream << std::hex << std::showbase << intValue;
					break;
				default:
					stream << intValue;
					break;
				}
			}
			else if constexpr (std::is_floating_point<T>())
			{
				switch (base)
				{
				case NumberFormat::DECIMAL:
					stream << value;
					break;
				case NumberFormat::BINARY:
				{
					// We iterate over the bytes in a float / double
					// and print them in order. That is difficult to do without
					// pointer arithmetic.
					stream << "0b";
					char* start = reinterpret_cast<char*>(&value); // NOLINT
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
					// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
					for (char* it = start + sizeof(T) - 1; it >= start; --it)
#else
					// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
					for (char* it = start; it != start + sizeof(T); ++it)
#endif
					{
						stream << std::bitset<byteSize>(*it);
					}
					break;
				}
				case NumberFormat::HEXADECIMAL:
					stream << std::hexfloat << value;
					break;
				default:
					stream << value;
					break;
				}
			}
			else
			{
				stream << value;
			}
			return stream.str();
		}

	private:
		static const size_t byteSize = 8;
	};

	template<std::size_t N>
	class EtherCATTypeStringFormatter<std::bitset<N>>
	{
	public:
		static std::string asString(std::bitset<N> value, NumberFormat base)
		{
			std::stringstream stream;
			switch (base)
			{
			case NumberFormat::DECIMAL:
				stream << value.to_ulong();
				break;
			case NumberFormat::BINARY:
				stream << "0b" << value;
				break;
			case NumberFormat::HEXADECIMAL:
				stream << "0x" << std::hex;
				for (int i = std::ceil(N / nybbleSize) - 1; i >= 0; --i)
				{
					stream << getNybble(value, i).to_ulong();
				}
				break;
			default:
				stream << "0b" << value;
				break;
			}
			return stream.str();
		}

	private:
		static constexpr float nybbleSize = 4.0;
		static constexpr size_t nybbleMask = 0xF;

		static std::bitset<N> getNybble(std::bitset<N> set, size_t i)
		{
			return (set >> (i * 4)) & std::bitset<N>(nybbleMask);
		}
	};

	template<>
	class EtherCATTypeStringFormatter<EtherCATDataType::OCTET_STRING>
	{
	public:
		static std::string asString(
		    const EtherCATDataType::OCTET_STRING& octString, NumberFormat base)
		{
			std::stringstream stream;
			bool first = true;
			for (uint8_t value : octString)
			{
				if (!first)
				{
					stream << " ";
				}
				first = false;
				switch (base)
				{
				case NumberFormat::DECIMAL:
					stream << static_cast<int>(value);
					break;
				case NumberFormat::BINARY:
					stream << "0b" << std::bitset<byteSize>(value);
					break;
				case NumberFormat::HEXADECIMAL:
					stream << std::hex << std::showbase << static_cast<int>(value);
					break;
				default:
					stream << static_cast<int>(value);
					break;
				}
			}
			return stream.str();
		}

	private:
		static const size_t byteSize = 8;
	};
} // namespace etherkitten::datatypes
