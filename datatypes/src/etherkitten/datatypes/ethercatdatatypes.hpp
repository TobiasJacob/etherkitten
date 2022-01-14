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
 * \brief Defines the fundamental data types used in the EtherCAT standard
 * along with some tools to handle them.
 */

#include <bitset>
#include <cstdint>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace etherkitten::datatypes
{

	/*!
	 * \brief Holds all the types we support via EtherCAT in a type-agnostic enum.
	 *
	 * The enum values correspond to the values defined in the standard.
	 * We use these to identify which types different DataObjects have.
	 */
	enum class EtherCATDataTypeEnum
	{
		BOOLEAN = 1,
		INTEGER8 = 2,
		INTEGER16 = 3,
		INTEGER32 = 4,
		UNSIGNED8 = 5,
		UNSIGNED16 = 6,
		UNSIGNED32 = 7,
		REAL32 = 8,
		VISIBLE_STRING = 9,
		OCTET_STRING = 10,
		UNICODE_STRING = 11,
		TIME_OF_DAY = 12,
		TIME_DIFFERENCE = 13,
		INTEGER24 = 16,
		REAL64 = 17,
		INTEGER64 = 21,
		UNSIGNED24 = 22,
		UNSIGNED64 = 27,
		BYTE = 30,
		BIT1 = 48,
		BIT2 = 49,
		BIT3 = 50,
		BIT4 = 51,
		BIT5 = 52,
		BIT6 = 53,
		BIT7 = 54,
		BIT8 = 55,
	};

	/*!
	 * \brief Holds all the types we support via EtherCAT as their C++ equivalents
	 * along with some tools to handle them.
	 *
	 * We use the types defined here to actually move the data around,
	 * and the templates to perform type-agnostic operations.
	 */
	namespace EtherCATDataType
	{
		using BOOLEAN = bool;
		using INTEGER8 = int8_t;
		using INTEGER16 = int16_t;
		using INTEGER32 = int32_t;
		using UNSIGNED8 = uint8_t;
		using UNSIGNED16 = uint16_t;
		using UNSIGNED32 = uint32_t;
		using REAL32 = float;
		using VISIBLE_STRING = std::string;
		using OCTET_STRING = std::vector<uint8_t>;
		using UNICODE_STRING = std::string;
		using TIME_OF_DAY = std::bitset<48>; // NOLINT
		using TIME_DIFFERENCE = uint64_t;
		using INTEGER24 = std::bitset<24>; // NOLINT
		using REAL64 = double;
		using INTEGER64 = int64_t;
		using UNSIGNED24 = std::bitset<24>; // NOLINT
		using UNSIGNED64 = uint64_t;
		using BYTE = uint8_t;
		using BIT1 = std::bitset<1>;
		using BIT2 = std::bitset<2>;
		using BIT3 = std::bitset<3>;
		using BIT4 = std::bitset<4>;
		using BIT5 = std::bitset<5>; // NOLINT
		using BIT6 = std::bitset<6>; // NOLINT
		using BIT7 = std::bitset<7>; // NOLINT
		using BIT8 = std::bitset<8>; // NOLINT

		/*!
		 * \brief Defines the length of the various EtherCATDataTypes in bits.
		 *
		 * If a type does not have a well-defined length, `bitLength<T> == -1`.
		 *
		 * Note that `bitLength<T> != sizeof(T) * 8` in general.
		 * \tparam T the EtherCATDataType to get the length of
		 */
		template<typename T>
		constexpr int bitLength = -1;
		template<>
		inline constexpr int bitLength<BOOLEAN> = 8;
		template<>
		inline constexpr int bitLength<INTEGER8> = 8;
		template<>
		inline constexpr int bitLength<INTEGER16> = 16;
		template<>
		inline constexpr int bitLength<INTEGER32> = 32;
		template<>
		inline constexpr int bitLength<UNSIGNED8> = 8;
		template<>
		inline constexpr int bitLength<UNSIGNED16> = 16;
		template<>
		inline constexpr int bitLength<UNSIGNED32> = 32;
		template<>
		inline constexpr int bitLength<REAL32> = 32;
		template<>
		inline constexpr int bitLength<VISIBLE_STRING> = -1;
		template<>
		inline constexpr int bitLength<OCTET_STRING> = -1;
		template<>
		inline constexpr int bitLength<TIME_OF_DAY> = 48;
		template<>
		inline constexpr int bitLength<INTEGER24> = 24; // NOLINT
		template<>
		inline constexpr int bitLength<REAL64> = 64;
		template<>
		inline constexpr int bitLength<INTEGER64> = 64;
		template<>
		inline constexpr int bitLength<UNSIGNED64> = 64;
		template<>
		inline constexpr int bitLength<BIT1> = 1;
		template<>
		inline constexpr int bitLength<BIT2> = 2;
		template<>
		inline constexpr int bitLength<BIT3> = 3;
		template<>
		inline constexpr int bitLength<BIT4> = 4;
		template<>
		inline constexpr int bitLength<BIT5> = 5; // NOLINT
		template<>
		inline constexpr int bitLength<BIT6> = 6; // NOLINT
		template<>
		inline constexpr int bitLength<BIT7> = 7; // NOLINT
		template<>
		inline constexpr int bitLength<BIT8> = 8; // NOLINT

		namespace mapInternal
		{
			/* This is a helper function to make the dataTypeMaps somewhat more legible. */
			template<template<EtherCATDataTypeEnum E, typename... Args> typename Func,
			    EtherCATDataTypeEnum E, typename... Argsout>
			std::pair<EtherCATDataTypeEnum, typename Func<E, Argsout...>::product_t> makePair()
			{
				return std::make_pair<EtherCATDataTypeEnum,
				    typename Func<E, Argsout...>::product_t>(E, Func<E, Argsout...>::eval());
			}

		} // namespace mapInternal

		/*!
		 * \brief Can be used to map from an EtherCATDataTypeEnum to an arbitrary type,
		 * including functions.
		 *
		 * This map does not include the string types defined in the EtherCAT standard.
		 *
		 * To use this map, define a template class (Func) that takes any number of
		 * template parameters. The first parameter must always be of the type
		 * EtherCATDataTypeEnum, the others must be typenames. Any additional parameters
		 * will be satisfied by Argsout on instantiation.
		 *
		 * IMPORTANT: If Func needs no parameters aside from the EtherCATDataType enum
		 * parameter, you must still give it at least one more template parameter,
		 * preferably like this: `template<EtherCATDataTypeEnum E, typename...>`.
		 * This is necessary because parameter packs whose valid instantiations
		 * all have a length of 0 are ill-formed.
		 *
		 * Func must define a dependant type `product_t`, which will be the type of
		 * the map's values on instantiation. `product_t` must NOT depend on the
		 * first EtherCATDataTypeEnum template parameter of Func, but may depend
		 * on the others.
		 *
		 * Additionally, Func must define a static function `eval()` that takes
		 * no arguments and returns a `product_t`. The result of calling `eval()`
		 * on the instantiations of Func in this map will determine the map's values.
		 *
		 * Here's an example template that can be used as a base for Func:
		 * ```
		 * template<EtherCATDataTypeEnum E, typename...>
		 * class Foo
		 * {
		 * public:
		 *     using product_t = <insert type here>;
		 *
		 *     static product_t eval()
		 *     {
		 *         return <whatever you need>;
		 *     }
		 * };
		 * ```
		 * \tparam Func the template that will generate the desired values
		 * \tparam Argsout the arguments to instantiate Func with
		 */
		template<template<EtherCATDataTypeEnum E, typename... Args> typename Func,
		    typename... Argsout>
		const std::unordered_map<EtherCATDataTypeEnum,
		    typename Func<EtherCATDataTypeEnum::UNSIGNED8, Argsout...>::product_t>
		    dataTypeMap{
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::BOOLEAN, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::INTEGER8, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::INTEGER16, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::INTEGER32, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::UNSIGNED8, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::UNSIGNED16, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::UNSIGNED32, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::REAL32, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::TIME_OF_DAY, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::TIME_DIFFERENCE, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::INTEGER24, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::REAL64, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::INTEGER64, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::UNSIGNED24, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::UNSIGNED64, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::BYTE, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::BIT1, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::BIT2, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::BIT3, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::BIT4, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::BIT5, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::BIT6, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::BIT7, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::BIT8, Argsout...>(),
		    };

		/*!
		 * \brief Can be used to map from an EtherCATDataTypeEnum to an arbitrary type,
		 * including functions.
		 *
		 * This map additionally includes the string types defined in the EtherCAT
		 * standard.
		 *
		 * To use this map, define a template class (Func) that takes any number of
		 * template parameters. The first parameter must always be of the type
		 * EtherCATDataTypeEnum, the others must be typenames. Any additional parameters
		 * will be satisfied by Argsout on instantiation.
		 *
		 * IMPORTANT: If Func needs no parameters aside from the EtherCATDataType enum
		 * parameter, you must still give it at least one more template parameter,
		 * preferably like this: `template<EtherCATDataTypeEnum E, typename...>`.
		 * This is necessary because parameter packs whose valid instantiations
		 * all have a length of 0 are ill-formed.
		 *
		 * Func must define a dependant type `product_t`, which will be the type of
		 * the map's values on instantiation. `product_t` must NOT depend on the
		 * first EtherCATDataTypeEnum template parameter of Func, but may depend
		 * on the others.
		 *
		 * Additionally, Func must define a static function `eval()` that takes
		 * no arguments and returns a `product_t`. The result of calling `eval()`
		 * on the instantiations of Func in this map will determine the map's values.
		 *
		 * Here's an example template that can be used as a base for Func:
		 * ```
		 * template<EtherCATDataTypeEnum E, typename...>
		 * class Foo
		 * {
		 * public:
		 *     using product_t = <insert type here>;
		 *
		 *     static product_t eval()
		 *     {
		 *         return <whatever you need>;
		 *     }
		 * };
		 * ```
		 * \tparam Func the template that will generate the desired values
		 * \tparam Argsout the arguments to instantiate Func with
		 */
		template<template<EtherCATDataTypeEnum E, typename... Args> typename Func,
		    typename... Argsout>
		const std::unordered_map<EtherCATDataTypeEnum,
		    typename Func<EtherCATDataTypeEnum::UNSIGNED8, Argsout...>::product_t>
		    dataTypeMapWithStrings{
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::BOOLEAN, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::INTEGER8, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::INTEGER16, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::INTEGER32, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::UNSIGNED8, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::UNSIGNED16, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::UNSIGNED32, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::REAL32, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::TIME_OF_DAY, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::TIME_DIFFERENCE, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::INTEGER24, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::REAL64, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::INTEGER64, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::UNSIGNED24, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::UNSIGNED64, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::VISIBLE_STRING, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::OCTET_STRING, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::UNICODE_STRING, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::BYTE, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::BIT1, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::BIT2, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::BIT3, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::BIT4, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::BIT5, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::BIT6, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::BIT7, Argsout...>(),
			    mapInternal::makePair<Func, EtherCATDataTypeEnum::BIT8, Argsout...>(),
		    };
	} // namespace EtherCATDataType

	using EtherCATDataType::dataTypeMap;
	using EtherCATDataType::dataTypeMapWithStrings;

	/*!
	 * \brief Maps from EtherCATDataTypeEnum to EtherCATDataType.
	 * \tparam E the EtherCATDataTypeEnum to translate to an EtherCATDataType
	 */
	template<EtherCATDataTypeEnum E>
	struct TypeMap
	{
	};

	// clang-format off
	template<>
	struct TypeMap<EtherCATDataTypeEnum::BOOLEAN> { using type = EtherCATDataType::BOOLEAN; };
	template<>
	struct TypeMap<EtherCATDataTypeEnum::INTEGER8> { using type = EtherCATDataType::INTEGER8; };
	template<>
	struct TypeMap<EtherCATDataTypeEnum::INTEGER16> { using type = EtherCATDataType::INTEGER16; };
	template<>
	struct TypeMap<EtherCATDataTypeEnum::INTEGER32> { using type = EtherCATDataType::INTEGER32; };
	template<>
	struct TypeMap<EtherCATDataTypeEnum::UNSIGNED8> { using type = EtherCATDataType::UNSIGNED8; };
	template<>
	struct TypeMap<EtherCATDataTypeEnum::UNSIGNED16> { using type = EtherCATDataType::UNSIGNED16; };
	template<>
	struct TypeMap<EtherCATDataTypeEnum::UNSIGNED32> { using type = EtherCATDataType::UNSIGNED32; };
	template<>
	struct TypeMap<EtherCATDataTypeEnum::UNSIGNED64> { using type = EtherCATDataType::UNSIGNED64; };
	template<>
	struct TypeMap<EtherCATDataTypeEnum::REAL32> { using type = EtherCATDataType::REAL32; };
	template<>
	struct TypeMap<EtherCATDataTypeEnum::VISIBLE_STRING> { using type = EtherCATDataType::VISIBLE_STRING; };
	template<>
	struct TypeMap<EtherCATDataTypeEnum::OCTET_STRING> { using type = EtherCATDataType::OCTET_STRING; };
	template<>
	struct TypeMap<EtherCATDataTypeEnum::UNICODE_STRING> { using type = EtherCATDataType::UNICODE_STRING; };
	template<>
	struct TypeMap<EtherCATDataTypeEnum::TIME_OF_DAY> { using type = EtherCATDataType::TIME_OF_DAY; };
	template<>
	struct TypeMap<EtherCATDataTypeEnum::TIME_DIFFERENCE> { using type = EtherCATDataType::TIME_DIFFERENCE; };
	template<>
	struct TypeMap<EtherCATDataTypeEnum::INTEGER24> { using type = EtherCATDataType::INTEGER24; };
	template<>
	struct TypeMap<EtherCATDataTypeEnum::REAL64> { using type = EtherCATDataType::REAL64; };
	template<>
	struct TypeMap<EtherCATDataTypeEnum::INTEGER64> { using type = EtherCATDataType::INTEGER64; };
	template<>
	struct TypeMap<EtherCATDataTypeEnum::UNSIGNED24> { using type = EtherCATDataType::UNSIGNED24; };
	template<>
	struct TypeMap<EtherCATDataTypeEnum::BYTE> { using type = EtherCATDataType::BYTE; };
	template<>
	struct TypeMap<EtherCATDataTypeEnum::BIT1> { using type = EtherCATDataType::BIT1; };
	template<>
	struct TypeMap<EtherCATDataTypeEnum::BIT2> { using type = EtherCATDataType::BIT2; };
	template<>
	struct TypeMap<EtherCATDataTypeEnum::BIT3> { using type = EtherCATDataType::BIT3; };
	template<>
	struct TypeMap<EtherCATDataTypeEnum::BIT4> { using type = EtherCATDataType::BIT4; };
	template<>
	struct TypeMap<EtherCATDataTypeEnum::BIT5> { using type = EtherCATDataType::BIT5; };
	template<>
	struct TypeMap<EtherCATDataTypeEnum::BIT6> { using type = EtherCATDataType::BIT6; };
	template<>
	struct TypeMap<EtherCATDataTypeEnum::BIT7> { using type = EtherCATDataType::BIT7; };
	template<>
	struct TypeMap<EtherCATDataTypeEnum::BIT8> { using type = EtherCATDataType::BIT8; };
	// clang-format on

	template<typename T>
	class is_bitset : public std::false_type // NOLINT(readability-identifier-naming)
	{
	};

	template<size_t N>
	class is_bitset<std::bitset<N>> : public std::true_type
	{
	};

} // namespace etherkitten::datatypes
