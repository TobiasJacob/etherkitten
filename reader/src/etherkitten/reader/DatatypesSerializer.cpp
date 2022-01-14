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


#include "DatatypesSerializer.hpp"

#include <functional>
#include <type_traits>

namespace etherkitten::reader
{
	/*!
	 * \brief Calculates the length in bytes of any value of an EtherCATDataType.
	 *
	 * Depending on the type, this may be a constant or a runtime value (e.g. int8_t vs
	 * std::string).
	 *
	 * To be used with the dataTypeMaps.
	 * \tparam E the type of the value to calculate the length of
	 */
	template<datatypes::EtherCATDataTypeEnum E, typename...>
	class ByteLengthCalculator
	{
	public:
		using product_t = std::function<size_t(std::any)>;

		static product_t eval()
		{
			return [](std::any data) -> size_t {
				if constexpr (E == datatypes::EtherCATDataTypeEnum::OCTET_STRING)
				{
					return std::any_cast<typename datatypes::TypeMap<E>::type&>(data).size() + 4;
				}
				else if constexpr (E == datatypes::EtherCATDataTypeEnum::VISIBLE_STRING
				    || E == datatypes::EtherCATDataTypeEnum::UNICODE_STRING)
				{
					return std::any_cast<typename datatypes::TypeMap<E>::type&>(data).size() + 1;
				}
				else if constexpr (E == datatypes::EtherCATDataTypeEnum::INTEGER24
				    || E == datatypes::EtherCATDataTypeEnum::UNSIGNED24)
				{
					return 4;
				}
				else if constexpr (E == datatypes::EtherCATDataTypeEnum::TIME_OF_DAY)
				{
					return 8;
				}
				else
				{
					static constexpr size_t byteSize = 8;
					size_t bitLength = datatypes::EtherCATDataType::bitLength<
					    typename datatypes::TypeMap<E>::type>;
					if (bitLength < byteSize)
					{
						return 1;
					}
					return bitLength / byteSize;
				}
			};
		}
	};

	uint64_t DatatypesSerializer::getLength(datatypes::EtherCATDataTypeEnum dataType, std::any data)
	{
		return datatypes::dataTypeMapWithStrings<ByteLengthCalculator>.at(dataType)(data);
	}

	/*!
	 * \brief Serializes any EtherCATDataType from a std::any into binary.
	 *
	 * To be used with the dataTypeMaps.
	 * \tparam E the type to serialize
	 */
	template<datatypes::EtherCATDataTypeEnum E, typename...>
	class Serializer
	{
	public:
		using product_t = std::function<void(std::any, Serialized&)>;

		static product_t eval()
		{
			return [](std::any data, Serialized& ser) {
				using Type = typename datatypes::TypeMap<E>::type;
				if constexpr (std::is_same<Type, std::bitset<24>>())
				{
					ser.write<uint32_t>(std::any_cast<Type&>(data).to_ulong(), 0);
				}
				else if constexpr (std::is_same<Type, std::bitset<48>>())
				{
					ser.write<uint64_t>(std::any_cast<Type&>(data).to_ulong(), 0);
				}
				else if constexpr (datatypes::is_bitset<Type>())
				{
					ser.write<uint8_t>(std::any_cast<Type&>(data).to_ulong(), 0);
				}
				else if constexpr (std::is_same<Type, std::vector<uint8_t>>())
				{
					auto& vec = std::any_cast<Type&>(data);
					ser.write<uint32_t>(vec.size(), 0);
					size_t off = 4;
					for (size_t i = 0; i < vec.size(); ++i)
					{
						ser.write<uint8_t>(vec.at(i), i + off);
					}
				}
				else
				{
					ser.write<Type>(std::any_cast<Type>(data), 0);
				}
			};
		}
	};

	void DatatypesSerializer::serialize(
	    datatypes::EtherCATDataTypeEnum type, std::any data, Serialized& ser)
	{
		datatypes::dataTypeMapWithStrings<Serializer>.at(type)(data, ser);
	}

	/*!
	 * \brief Parses any EtherCATDataType from binary into a std::any.
	 *
	 * To be used with the dataTypeMaps.
	 * \tparam E the type to parse
	 */
	template<datatypes::EtherCATDataTypeEnum E, typename...>
	class Parser
	{
	public:
		using product_t = std::function<std::any(Serialized&)>;

		static product_t eval()
		{
			return [](Serialized& ser) -> std::any {
				using Type = typename datatypes::TypeMap<E>::type;
				if constexpr (std::is_same<Type, std::bitset<24>>())
				{
					return std::make_any<Type>(Type(ser.read<uint32_t>(0)));
				}
				else if constexpr (std::is_same<Type, std::bitset<48>>())
				{
					return std::make_any<Type>(Type(ser.read<uint64_t>(0)));
				}
				else if constexpr (datatypes::is_bitset<Type>())
				{
					return std::make_any<Type>(Type(ser.read<uint8_t>(0)));
				}
				else if constexpr (std::is_same<Type, std::vector<uint8_t>>())
				{
					uint32_t size = ser.read<uint32_t>(0);
					std::vector<uint8_t> data;
					data.reserve(size);
					for (size_t i = 0; i < size; ++i)
					{
						data.push_back(ser.read<uint8_t>(4 + i));
					}
					return std::make_any<std::vector<uint8_t>>(data);
				}
				else
				{
					return std::make_any<Type>(ser.read<Type>(0));
				}
			};
		}
	};

	std::any DatatypesSerializer::parse(datatypes::EtherCATDataTypeEnum datatype, Serialized& ser)
	{
		return datatypes::dataTypeMapWithStrings<Parser>.at(datatype)(ser);
	}
} // namespace etherkitten::reader
