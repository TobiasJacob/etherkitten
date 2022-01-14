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

#include <etherkitten/datatypes/DataObjectVisitor.hpp>
#include <etherkitten/datatypes/EtherCATTypeParser.hpp>
#include <etherkitten/datatypes/dataobjects.hpp>
#include <etherkitten/reader/EtherKitten.hpp>
#include <memory>

namespace etherkitten::controller
{
	/**
	 * \brief A DataObjectVisitor that can supply a NewestValueView for the DataObject it visits.
	 */
	class NewestValueViewVisitor : public datatypes::DataObjectVisitor
	{
	private:
		reader::EtherKitten& etherKitten;
		std::unique_ptr<datatypes::AbstractNewestValueView> newestValueView;

	public:
		/*!
		 * \brief Create a NewestValueViewVisitor that can supply a NewestValueView for the
		 * DataObject it visits.
		 * \param[in]  etherKitten an EtherKitten instance to get the NewestValueView from
		 */
		NewestValueViewVisitor(reader::EtherKitten& etherKitten);
		~NewestValueViewVisitor() override;
		//! \copydoc datatypes::DataObjectVisitor::handleCoE()
		void handleCoE(const datatypes::CoEObject& object) override;
		//! \copydoc datatypes::DataObjectVisitor::handlePDO()
		void handlePDO(const datatypes::PDO& object) override;
		//! \copydoc datatypes::DataObjectVisitor::handleErrorStatistic()
		void handleErrorStatistic(const datatypes::ErrorStatistic& statistic) override;
		//! \copydoc datatypes::DataObjectVisitor::handleRegister()
		void handleRegister(const datatypes::Register& reg) override;
		/**
		 * \brief Return the NewestValueView for the visited object, if possible. Should only be
		 * called if hasNewestValueView() evaluated to true.
		 * \return a NewestValueView for the handled DataObject, if the handled DataObject and
		 * EtherKitten supported it.
		 * \exception std::runtime_error if no NewestValueView is available
		 */
		std::unique_ptr<datatypes::AbstractNewestValueView> getNewestValueView();
		/**
		 * \brief Return whether a NewestValueView can be returned by getNewestValueView().
		 * \retval  true a NewestValueView can be returned
		 * \retval  false no NewestValueView can be returned
		 */
		bool hasNewestValueView();
	};

	/**
	 * \brief A DataObjectVisitor that can supply a DataView for the DataObject it visits.
	 */
	class DataViewVisitor : public datatypes::DataObjectVisitor
	{
	private:
		reader::EtherKitten& etherKitten;
		datatypes::TimeSeries timeSeries;
		std::shared_ptr<datatypes::AbstractDataView> dataView;

	public:
		/*!
		 * \brief Create a DataViewVisitor that can supply a DataView for the
		 * DataObject it visits.
		 * \param[in]  etherKitten an EtherKitten instance to get the DataView from
		 * \param[in]  timeSeries  the TimeSeries the DataView should have
		 */
		DataViewVisitor(reader::EtherKitten& etherKitten, datatypes::TimeSeries timeSeries);
		~DataViewVisitor() override;
		/*!
		 * \copydoc datatypes::DataObjectVisitor::handleCoE()
		 * \details Does nothing except invalidating the result of any previously
		 * handled DataObject.
		 */
		void handleCoE(const datatypes::CoEObject& object) override;
		//! \copydoc datatypes::DataObjectVisitor::handlePDO()
		void handlePDO(const datatypes::PDO& object) override;
		//! \copydoc datatypes::DataObjectVisitor::handleErrorStatistic()
		void handleErrorStatistic(const datatypes::ErrorStatistic& statistic) override;
		//! \copydoc datatypes::DataObjectVisitor::handleRegister()
		void handleRegister(const datatypes::Register& reg) override;
		/**
		 * \brief Return the DataView for the visited object, if possible. Should only be
		 * called if hasDataView() evaluated to true.
		 * \return a DataView for the handled DataObject, if the handled DataObject and
		 * EtherKitten supported it.
		 * \exception std::runtime_error if no AbstractDataView is available
		 */
		std::shared_ptr<datatypes::AbstractDataView> getDataView();
		/**
		 * \brief Return whether an AbstractDataView can be returned by getDataView().
		 * \retval  true a AbstractDataView can be returned
		 * \retval  false no AbstractDataView can be returned
		 */
		bool hasDataView();
	};

	/**
	 * \brief a DataObjectVisitor that can write a value for the DataObject it visits.
	 */
	class WriteVisitor : public datatypes::DataObjectVisitor
	{
	private:
		reader::EtherKitten& etherKitten;
		std::string value;
		std::unique_ptr<datatypes::AbstractDataPoint> dataPoint;
		void convertValueFor(const datatypes::DataObject& object);

	public:
		/*!
		 * \brief Create a WriteVisitor that can write a value for the
		 * DataObject it visits. May produce a ParseException when visiting if value
		 * is not parseable into the necessary type.
		 * \param[in] etherKitten an EtherKitten instance to write the value with
		 * \param[in] value the value to write
		 */
		WriteVisitor(reader::EtherKitten& etherKitten, std::string value);
		~WriteVisitor() override;
		/*!
		 * \copydoc datatypes::DataObjectVisitor::handleCoE()
		 * \exception ParseException if value is not parseable into the necessary type
		 */
		void handleCoE(const datatypes::CoEObject& object) override;
		/*!
		 * \copydoc datatypes::DataObjectVisitor::handlePDO()
		 * \exception ParseException if value is not parseable into the necessary type
		 */
		void handlePDO(const datatypes::PDO& object) override;
		/*!
		 * \copydoc datatypes::DataObjectVisitor::handleErrorStatistic()
		 * \details Does nothing.
		 */
		void handleErrorStatistic(const datatypes::ErrorStatistic& statistic) override;
		/*!
		 * \copydoc datatypes::DataObjectVisitor::handleRegister()
		 * \details Does nothing.
		 */
		void handleRegister(const datatypes::Register& reg) override;

		/**
		 * \brief Parses and inserts a string into an AbstractDataPoint
		 * \tparam  T the type to parse for
		 */
		template<typename T>
		struct DataPointInserter
		{
			/**
			 * \brief Parse a string and insert the parsed value into the given AbstractDataPoint.
			 * \param[in]  dataPoint the AbstractDataPoint to insert the parsed value into
			 * \param[in]  str the string to parse and insert
			 * \exception ParseException if str is not parseable (into the specified type)
			 */
			static void insert(
			    std::unique_ptr<datatypes::AbstractDataPoint>& dataPoint, std::string str)
			{
				dataPoint.reset(new datatypes::DataPoint<T>(
				    datatypes::EtherCATTypeParser<T>::parse(str), datatypes::now()));
			}
		};
	};
} // namespace etherkitten::controller
