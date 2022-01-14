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

#include <QString>
#include <etherkitten/datatypes/EtherCATTypeStringFormatter.hpp>
#include <etherkitten/datatypes/dataobjects.hpp>
#include <etherkitten/datatypes/datapoints.hpp>
#include <etherkitten/datatypes/esidata.hpp>
#include <etherkitten/datatypes/ethercatdatatypes.hpp>
#include <memory>
#include <string>

namespace etherkitten::gui
{

	/*!
	 * \brief Represents a single item of ESI data in
	 * a more usable form for the GUI.
	 */
	class AbstractESIDataWrapper
	{
	public:
		virtual ~AbstractESIDataWrapper() {}
		/*!
		 * \brief Return the name of the object.
		 * \return  The name of the object.
		 */
		virtual std::string getName() const = 0;
		/*!
		 * \brief Return the tooltip for the object.
		 * \return  The tooltip for the object.
		 */
		virtual QString getTooltip() const = 0;
		/*!
		 * \brief Convert the value of the object to a string.
		 * \param base The base into which the value should be converted.
		 * \return The converted string.
		 */
		virtual std::string asString(datatypes::NumberFormat base) const = 0;
		/*!
		 * \brief Check if the value of this object is numeric.
		 * \return true, if the value is numeric, false otherwise.
		 */
		virtual bool isNumeric() const = 0;
	};

	template<typename T>
	class ESIDataWrapper : public AbstractESIDataWrapper
	{
	public:
		ESIDataWrapper(std::string name, T value, QString tooltip)
		    : value(value)
		    , name(name)
		    , tooltip(tooltip)
		{
		}
		std::string getName() const { return name; }
		QString getTooltip() const { return tooltip; }
		std::string asString(datatypes::NumberFormat base) const
		{
			return datatypes::EtherCATTypeStringFormatter<T>::asString(value, base);
		}
		bool isNumeric() const { return std::is_integral<T>() || std::is_floating_point<T>(); }

	private:
		const T value;
		const std::string name;
		const QString tooltip;
	};

	template<std::size_t N>
	class ESIDataWrapper<std::bitset<N>> : public AbstractESIDataWrapper
	{
	public:
		ESIDataWrapper(std::string name, std::bitset<N> value, QString tooltip)
		    : value(value)
		    , name(name)
		    , tooltip(tooltip)
		{
		}
		std::string getName() const { return name; }
		QString getTooltip() const { return tooltip; }
		std::string asString(datatypes::NumberFormat base) const
		{
			return datatypes::EtherCATTypeStringFormatter<std::bitset<N>>::asString(value, base);
		}
		bool isNumeric() const { return true; }

	private:
		const std::bitset<N> value;
		const std::string name;
		const QString tooltip;
	};

	/*!
	 * \brief Adapts the ESI data for easier usage by the GUI.
	 */
	class ESIAdapter
	{
	public:
		/*!
		 * \brief Convert the ESI data to a wrapper form for the GUI.
		 * \param data The ESIData from the backend.
		 * \return A list of wrapper objects for the GUI.
		 */
		static std::vector<std::unique_ptr<AbstractESIDataWrapper>> convertESIData(
		    const datatypes::ESIData& data);

	private:
		static void addPDO(std::vector<std::unique_ptr<AbstractESIDataWrapper>>& entries,
		    const std::vector<datatypes::ESIPDOObject>& pdos, const datatypes::ESIData& data,
		    std::string prefix);
		static std::string getString(size_t index, const datatypes::ESIStrings& strings);
	};
} // namespace etherkitten::gui
