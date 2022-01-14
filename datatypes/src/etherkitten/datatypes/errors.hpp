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
 * \brief Defines types used for error handling.
 */

#include <iostream>
#include <string>
#include <utility>

#include "datapoints.hpp"

namespace etherkitten::datatypes
{

	/*!
	 * \brief The ErrorSeverity enum encodes different error severities
	 * that can occur in ErrorMessage.
	 */
	enum class ErrorSeverity
	{
		LOW = 0, /*!< \brief LOW errors do not affect continued operation. */
		MEDIUM = 1, /*!< \brief MEDIUM errors allow only limited continued operation. */
		FATAL = 2, /*!< \brief FATAL errors do not allow continued operation. */
	};

	/*!
	 * \brief The ErrorMessage class holds a single message to asynchronously communicate
	 * an error from a low level routine.
	 *
	 * An ErrorMessage can be associated with one, two, or no EtherCAT slaves.
	 * If less than two slaves are associated with this message, one or two of the
	 * `getAssociatedSlaves()` pair will be equal to `std::numeric_limits<unsigned int>::max()`.
	 */
	class ErrorMessage
	{
	public:
		/*!
		 * \brief Construct a new ErrorMessage with no associated slaves.
		 *
		 * \param message the message to store in this ErrorMessage
		 * \param severity the severity of this error
		 */
		ErrorMessage(std::string&& message, ErrorSeverity severity);

		/*!
		 * \brief Construct a new ErrorMessage with one associated slave.
		 *
		 * \param message the message to store in this ErrorMessage
		 * \param slave which EtherCAT slave is associated with this error
		 * \param severity the severity of this error
		 */
		ErrorMessage(std::string&& message, unsigned int slave, ErrorSeverity severity);

		/*!
		 * \brief Construct a new ErrorMessage with two associated slaves.
		 *
		 * \param message the message to store in this ErrorMessage
		 * \param slaves which EtherCAT slaves are associated with this error
		 * \param severity the severity of this error
		 */
		ErrorMessage(std::string&& message, std::pair<unsigned int, unsigned int> slaves,
		    ErrorSeverity severity);

		/*!
		 * \brief Get the message contained in this ErrorMessage.
		 * \return the message
		 */
		const std::string& getMessage() const;

		/*!
		 * \brief Get the slaves associated with this ErrorMessage.
		 *
		 * If less than two slaves are associated with this message, one or two of the
		 * pair will be equal to `std::numeric_limits<unsigned int>::max()`.
		 * \return the slaves associated with this ErrorMessage
		 */
		const std::pair<unsigned int, unsigned int>& getAssociatedSlaves() const;

		/*!
		 * \brief Get the severity of this error.
		 * \return the severity of this error
		 */
		ErrorSeverity getSeverity() const;

		/*!
		 * \brief Overload the '<<' operator so the EtherCATTypeStringFormatter
		 * works with ErrorMessages. This adds a string representation of the
		 * ErrorMessage, consisting of the message, affected slaves, and severity,
		 *  to the given output stream.
		 * \param os The output stream.
		 * \param error The ErrorMessage.
		 * \return The same output stream, with the message, affected slaves,
		 * and severity of the ErrorMessage appended.
		 */
		friend std::ostream& operator<<(std::ostream& os, const ErrorMessage& error);

	private:
		std::string message;
		std::pair<unsigned int, unsigned int> slaves;
		ErrorSeverity severity;
	};

	/*!
	 * \brief Defines an iterator over a collection of ErrorMessages.
	 *
	 * This class fulfills the AbstractDataView interface with the exception of
	 * the removed `asDouble()`-method and the addition of `operator*`.
	 */
	class ErrorIterator // NOLINT(cppcoreguidelines-special-member-functions)
	{
	public:
		virtual ~ErrorIterator() {}

		/*!
		 * \brief Move to the next ErrorMessage in the queue.
		 *
		 * This must not be called when hasNext() would return false.
		 * \return A reference to this ErrorIterator.
		 */
		virtual ErrorIterator& operator++() = 0;

		/*!
		 * \brief Return whether there is another ErrorMessage in the queue.
		 * \return Whether there is another ErrorMessage in the queue.
		 */
		virtual bool hasNext() const = 0;

		/*!
		 * \brief Check whether this ErrorIterator points to no data.
		 *
		 * This method will never return true after ++ has been applied to this ErrorIterator
		 * at least once.
		 * \retval true iff this ErrorIterator may not be dereferenced
		 * \retval false iff this ErrorIterator may be dereferenced
		 */
		virtual bool isEmpty() const = 0;

		/*!
		 * \brief Return the actual DataPoint containing the ErrorMessage.
		 * \return The actual DataPoint containing the ErrorMessage.
		 */
		virtual DataPoint<ErrorMessage> operator*() const = 0;
	};

	/*!
	 * \brief Wraps an ErrorIterator, so that isEmpty always returns true initially
	 */
	class FirstEmptyErrorIterator : public ErrorIterator
	{
	public:
		/*!
		 * \brief Creates a new FirstEmptyErrorIterator that wraps around a shared ptr of an
		 * ErrorIterator
		 * \param errorIterator the ErrorIterator to wrap around
		 */
		FirstEmptyErrorIterator(std::shared_ptr<ErrorIterator> errorIterator);

		ErrorIterator& operator++() override;

		bool hasNext() const override;

		bool isEmpty() const override;

		DataPoint<ErrorMessage> operator*() const override;

	private:
		bool firstIsValid = false;
		std::shared_ptr<ErrorIterator> errorIterator;
	};

} // namespace etherkitten::datatypes
