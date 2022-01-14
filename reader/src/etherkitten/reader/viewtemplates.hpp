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
 * \brief Defines templates for working with Views over EtherCATDataTypes.
 */

#include <functional>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <variant>

#include <etherkitten/datatypes/dataobjects.hpp>
#include <etherkitten/datatypes/dataviews.hpp>
#include <etherkitten/datatypes/ethercatdatatypes.hpp>
#include <etherkitten/datatypes/time.hpp>

#include "NewestValueView.hpp"

namespace etherkitten::reader::bReader
{
	/*!
	 * \brief A variant that can hold SearchLists for all the potential EtherCAT registers.
	 * \tparam NodeSize the size of the SearchList nodes
	 */
	// clang-format off
	template<size_t NodeSize>
	using RegTypesVariant = std::variant<
		SearchList<datatypes::EtherCATDataType::UNSIGNED8, NodeSize>,
		SearchList<datatypes::EtherCATDataType::UNSIGNED16, NodeSize>,
		SearchList<datatypes::EtherCATDataType::UNSIGNED32, NodeSize>,
		SearchList<datatypes::EtherCATDataType::UNSIGNED64, NodeSize>
		>;
	// clang-format on

	/*!
	 * \brief Encodes a compile-time `size_t` value as a type.
	 *
	 * Get the value with `SizeT2Type<...>::value`.
	 * \tparam I the value to encode as a type.
	 */
	template<size_t I>
	struct SizeT2Type
	{
		enum : size_t
		{
			value = I
		};
	};

	/*!
	 * \brief Returns a NewestValueView with underlying type T and output type E.
	 *
	 * To be used with the dataTypeMaps.
	 * \tparam E the output type of the view
	 * \tparam T the underlying type of the view
	 * \tparam NodeSize the size of the nodes in the list as a SizeT2Type
	 */
	template<datatypes::EtherCATDataTypeEnum E, typename T, typename NodeSize>
	class NewestValueViewRetriever
	{
	public:
		using product_t = std::function<std::unique_ptr<datatypes::AbstractNewestValueView>(
		    SearchList<T, NodeSize::value>&, size_t, size_t)>;

		static product_t eval()
		{
			return [](SearchList<T, NodeSize::value>& list, size_t bitOffset,
			           size_t bitLength) -> std::unique_ptr<datatypes::AbstractNewestValueView> {
				return std::unique_ptr<datatypes::AbstractNewestValueView>(
				    new NewestValueView<typename SearchList<T, NodeSize::value>::contained,
				        NodeSize::value, typename datatypes::TypeMap<E>::type>(
				        list, bitOffset, bitLength, true));
			};
		}
	};

	/*!
	 * \brief Returns a DataView with underlying type T and output type E.
	 *
	 * To be used with the dataTypeMaps.
	 * \tparam E the output type of the view
	 * \tparam T the underlying type of the view
	 * \tparam NodeSize the size of the nodes in the list as a SizeT2Type
	 */
	template<datatypes::EtherCATDataTypeEnum E, typename T, typename NodeSize>
	class DataViewRetriever
	{
	public:
		using product_t = std::function<std::shared_ptr<datatypes::AbstractDataView>(
		    SearchList<T, NodeSize::value>&, datatypes::TimeSeries, size_t, size_t)>;

		static product_t eval()
		{
			return [](SearchList<T, NodeSize::value>& list, datatypes::TimeSeries time,
			           size_t bitOffset,
			           size_t bitLength) -> std::shared_ptr<datatypes::AbstractDataView> {
				return list.template getView<typename datatypes::TypeMap<E>::type>(
				    time, bitOffset, bitLength, true);
			};
		}
	};

	/*!
	 * \brief Make a NewestValueView or DataView for a register given a map of register lists.
	 *
	 * To be used with the dataTypeMaps.
	 * \tparam View the view to return. Must be either std::shared_ptr<datatypes::AbstractDataView>
	 * or std::unique_ptr<datatypes::AbstractNewestValueView>
	 * \tparam NodeSize the size of the nodes in the list as a SizeT2Type
	 * \param registerLists the lists to get the view from
	 * \param reg the Register to get the view for
	 * \param time the TimeSeries for the view
	 * \param slaveConfiguredAddress the address of the slave to get the view for
	 * \return a view of the requested type for the requested register
	 */
	template<typename View, typename NodeSize>
	View makeRegisterView(
	    std::unordered_map<uint16_t,
	        std::unordered_map<datatypes::RegisterEnum, RegTypesVariant<NodeSize::value>>>&
	        registerLists,
	    const datatypes::Register& reg, datatypes::TimeSeries time, uint16_t slaveConfiguredAddress)
	{
		static constexpr int twoByteMask = 0xFFFF;
		static constexpr int twoByteSize = 16;
		int registerAsInt = static_cast<int>(reg.getRegister());
		int registerAddress = registerAsInt & twoByteMask;
		int bitOffset = (registerAsInt - registerAddress) >> twoByteSize;
		size_t bitLength = datatypes::registerMap.at(reg.getRegister()).bitLength;
		datatypes::RegisterEnum addressAsRegister
		    = static_cast<datatypes::RegisterEnum>(registerAddress);
		return std::visit(
		    [&reg, time, bitOffset, bitLength](auto& arg) -> View {
			    if constexpr (std::is_same<View, std::shared_ptr<datatypes::AbstractDataView>>())
			    {
				    return datatypes::dataTypeMap<DataViewRetriever,
						typename std::remove_reference<decltype(arg)>::type::contained, NodeSize>.at(
				        reg.getType())(arg, time, bitOffset, bitLength);
			    }
			    else if constexpr (std::is_same<View,
			                           std::unique_ptr<datatypes::AbstractNewestValueView>>())
			    {
				    return datatypes::dataTypeMap<NewestValueViewRetriever,
						typename std::remove_reference<decltype(arg)>::type::contained, NodeSize>.at(
						reg.getType())(arg, bitOffset, bitLength);
			    }
		    },
		    registerLists[slaveConfiguredAddress][addressAsRegister]);
	}
} // namespace etherkitten::reader::bReader
