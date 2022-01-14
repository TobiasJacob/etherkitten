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
 * \brief Defines the Converter template, which converts between pre-defined types used in the
 * reader.
 */

#include <cstring>
#include <limits>
#include <memory>
#include <stdexcept>
#include <type_traits>

#include <etherkitten/datatypes/datapoints.hpp>
#include <etherkitten/datatypes/errors.hpp>

#include "IOMap.hpp"
#include "endianness.hpp"

namespace etherkitten::reader
{
	/*!
	 * \brief The Converter class converts pre-defined types from one to the other.
	 *
	 * Currently supported types are:
	 *  - all integer types for both Input and Output
	 *  - std::bitset<N> with `N <= 64` as Input and integer types as output
	 *  - std::string as Input, with Output as a type for which `static_cast<Output>(std::string)`
	 * is defined
	 *  - std::vector<uint8_t> as Input, with Output as a type for which
	 * `static_cast<Output>(std::vector<uint8_t>)` is defined
	 *  - datatypes::ErrorMessage as Input and Output
	 *  - double as Input and Output
	 *  - IOMap* as Input, with Output as any type in datatypes::EtherCATDataType except for
	 * std::string and std::vector<uint8_t>
	 * \tparam Type the type to convert from
	 * \tparam Output the type to convert to
	 */
	template<typename Type, typename Output>
	class Converter
	{
	public:
		/*!
		 * \brief Convert the input to the Output type while respecting the bit offset and length.
		 *
		 * For some input / output combinations, bitOffset, bitLength and flipBytes will be ignored.
		 * \param input the input to convert
		 * \param bitOffset the bit offset of the input
		 * \param bitLength the bit length of the input
		 * \param flipBytes whether to flip the bytes of the input for the output on big endian
		 * hosts
		 * \return the input converted to the Output type
		 */
		static Output shiftAndConvert(
		    Type input, size_t bitOffset, size_t bitLength, bool flipBytes)
		{
			Type bytesFlipped = flipBytes ? flipBytesIfBigEndianHost(input) : input;
			if (bitOffset == 0 && bitLength == 0)
			{
				return static_cast<Output>(bytesFlipped);
			}
			Type shifted;
			static constexpr size_t byteSize = 8;
			uint64_t bitMask = bitLength == sizeof(uint64_t) * byteSize
			    ? ~((uint64_t)0)
			    : (((uint64_t)1 << bitLength) - 1);
			if constexpr (datatypes::is_bitset<Type>())
			{
				shifted = (bytesFlipped >> bitOffset) & Type(bitMask);
			}
			else
			{
				shifted = (bytesFlipped >> bitOffset) & bitMask;
			}
			if constexpr (std::is_signed<Output>() && !std::is_floating_point<Output>())
			{
				Output max = std::numeric_limits<Output>::max();
				return shifted <= static_cast<Type>(max)
				    ? static_cast<Output>(shifted)
				    : static_cast<Output>(shifted - max - 1) + std::numeric_limits<Output>::min();
			}
			else
			{
				return static_cast<Output>(shifted);
			}
		}
	};

	template<typename Output>
	class Converter<std::string, Output>
	{
	public:
		static Output shiftAndConvert(
		    const std::string& input, size_t bitOffset, size_t bitLength, bool flipBytes)
		{
			(void)bitOffset;
			(void)bitLength;
			(void)flipBytes;

			return static_cast<Output>(input);
		}
	};

	template<typename Output>
	class Converter<std::vector<uint8_t>, Output>
	{
	public:
		static Output shiftAndConvert(
		    const std::vector<uint8_t>& input, size_t bitOffset, size_t bitLength, bool flipBytes)
		{
			(void)bitOffset;
			(void)bitLength;
			(void)flipBytes;

			return static_cast<Output>(input);
		}
	};

	template<>
	class Converter<datatypes::ErrorMessage, datatypes::ErrorMessage>
	{
	public:
		static datatypes::ErrorMessage shiftAndConvert(
		    datatypes::ErrorMessage input, size_t bitOffset, size_t bitLength, bool flipBytes)
		{
			(void)bitOffset;
			(void)bitLength;
			(void)flipBytes;

			return input;
		}
	};

	template<>
	class Converter<double, double>
	{
	public:
		static double shiftAndConvert(
		    double input, size_t bitOffset, size_t bitLength, bool flipBytes)
		{
			(void)bitOffset;
			(void)bitLength;

			return flipBytes ? flipBytesIfBigEndianHost(input) : input;
		}
	};

	template<typename Output>
	class Converter<IOMap*, Output>
	{
	public:
		static Output shiftAndConvert(
		    IOMap* input, size_t bitOffset, size_t bitLength, bool flipBytes)
		{
			if constexpr (std::is_same<Output, IOMap*>())
			{
				return input;
			}
			else
			{
				if (bitOffset == 0 && bitLength == 0)
				{
					throw std::runtime_error("Cannot return nothing from IOMap");
				}
				static constexpr size_t byteSize = 8;
				size_t startBit = bitOffset % byteSize;
				uint8_t* ioMapStart = input->ioMap; // NOLINT
				uint64_t result = 0;
				uint64_t bitMask = bitLength == sizeof(uint64_t) * byteSize
				    ? ~((uint64_t)0)
				    : (((uint64_t)1 << bitLength) - 1);
				std::memcpy(
				    &result, ioMapStart + (bitOffset / byteSize), sizeof(uint64_t)); // NOLINT
				uint64_t shifted = 0;
				if (startBit + bitLength > sizeof(uint64_t) * byteSize)
				{
					uint64_t eightByteOverhang
					    = (*(ioMapStart + (bitOffset / byteSize) + sizeof(uint64_t))) // NOLINT
					    & ((1 << startBit) - 1);
					shifted = ((result >> startBit) & bitMask)
					    | (eightByteOverhang << (sizeof(uint64_t) * byteSize - startBit));
				}
				else
				{
					shifted = (result >> startBit) & bitMask;
				}
				shifted = flipBytes ? flipBytesIfBigEndianHost(shifted) : shifted;
				if constexpr (std::is_signed<Output>() && !std::is_floating_point<Output>())
				{
					Output max = std::numeric_limits<Output>::max();
					return shifted <= static_cast<uint64_t>(max)
					    ? static_cast<Output>(shifted)
					    : static_cast<Output>(shifted - max - 1)
					        + std::numeric_limits<Output>::min();
				}
				else
				{
					return static_cast<Output>(shifted);
				}
			}
		}
	};
} // namespace etherkitten::reader
