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

#include <ostream>

#include "errors.hpp"
#include <limits>

namespace etherkitten::datatypes
{
	ErrorMessage::ErrorMessage(std::string&& message, ErrorSeverity severity)
	    : ErrorMessage(std::move(message), std::numeric_limits<unsigned int>::max(), severity)
	{
	}

	ErrorMessage::ErrorMessage(std::string&& message, unsigned int slave, ErrorSeverity severity)
	    : ErrorMessage(
	        std::move(message), { slave, std::numeric_limits<unsigned int>::max() }, severity)
	{
	}

	ErrorMessage::ErrorMessage(
	    std::string&& message, std::pair<unsigned int, unsigned int> slaves, ErrorSeverity severity)
	    : message(message)
	    , slaves(slaves)
	    , severity(severity)
	{
	}

	const std::string& ErrorMessage::getMessage() const { return this->message; }

	const std::pair<unsigned int, unsigned int>& ErrorMessage::getAssociatedSlaves() const
	{
		return this->slaves;
	}

	ErrorSeverity ErrorMessage::getSeverity() const { return this->severity; }

	std::ostream& operator<<(std::ostream& os, const ErrorMessage& error)
	{
		std::string severity;
		switch (error.severity)
		{
		case ErrorSeverity::LOW:
			severity = "Low";
			break;
		case ErrorSeverity::MEDIUM:
			severity = "Medium";
			break;
		case ErrorSeverity::FATAL:
			severity = "Fatal";
			break;
		default:
			severity = "Unknown";
			break;
		}
		unsigned int max = std::numeric_limits<unsigned int>::max();
		os << "Error message: " << error.message;
		if (error.slaves.first != max && error.slaves.second != max)
		{
			os << "; Slaves: " << error.slaves.first << ", " << error.slaves.second;
		}
		else if (error.slaves.first != max)
		{
			os << "; Slave: " << error.slaves.first;
		}
		os << "; Severity: " << severity;
		return os;
	}

	FirstEmptyErrorIterator::FirstEmptyErrorIterator(std::shared_ptr<ErrorIterator> errorIterator)
	    : errorIterator(errorIterator)
	{
		firstIsValid = !errorIterator->isEmpty();
	}
	ErrorIterator& FirstEmptyErrorIterator::operator++()
	{
		if (firstIsValid)
			firstIsValid = false;
		else
			++(*errorIterator);
		return *this;
	}

	bool FirstEmptyErrorIterator::hasNext() const
	{
		return errorIterator->hasNext() || firstIsValid;
	}

	bool FirstEmptyErrorIterator::isEmpty() const
	{
		return firstIsValid || errorIterator->isEmpty();
	}

	DataPoint<ErrorMessage> FirstEmptyErrorIterator::operator*() const
	{
		return errorIterator->operator*();
	}
} // namespace etherkitten::datatypes
