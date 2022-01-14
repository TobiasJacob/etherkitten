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
 * \brief Defines the DataObjectVisitor interface that can be used to implement
 * the visitor pattern on DataObjects.
 */

namespace etherkitten::datatypes
{

	class PDO;
	class CoEObject;
	class ErrorStatistic;
	class Register;

	/*!
	 * \brief The DataObjectVisitor class defines the interface for a visitor of a DataObject.
	 *
	 * This interface defines a method for every DataObject subclass. The methods
	 * are called by their corresponding DataObject subclass when the `acceptVisitor` method
	 * is called.
	 */
	class DataObjectVisitor // NOLINT(cppcoreguidelines-special-member-functions)
	{
	public:
		virtual ~DataObjectVisitor(){};

		/*!
		 * \brief Handle a PDO.
		 * \param object the PDO to handle
		 */
		virtual void handlePDO(const PDO& object) = 0;

		/*!
		 * \brief Handle a CoEObject.
		 * \param object the CoEObject to handle
		 */
		virtual void handleCoE(const CoEObject& object) = 0;

		/*!
		 * \brief Handle an ErrorStatistic.
		 * \param statistic the ErrorStatistic to handle
		 */
		virtual void handleErrorStatistic(const ErrorStatistic& statistic) = 0;

		/*!
		 * \brief Handle a Register.
		 * \param reg the Register to handle
		 */
		virtual void handleRegister(const Register& reg) = 0;
	};

} // namespace etherkitten::datatypes
