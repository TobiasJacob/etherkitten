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

#include <QObject>
#include <etherkitten/datatypes/DataObjectVisitor.hpp>
#include <etherkitten/datatypes/dataobjects.hpp>
#include <string>

namespace etherkitten::gui
{

	/*!
	 * \brief Creates tooltips for DataObjects.
	 */
	class TooltipFormatter : public datatypes::DataObjectVisitor
	{
	public:
		/*!
		 * \brief Construct a new TooltipFormatter.
		 */
		TooltipFormatter();
		/*!
		 * \brief Return the tooltip.
		 * \return The tooltip.
		 */
		QString getTooltip() const;
		void handlePDO(const datatypes::PDO& object) override;
		void handleCoE(const datatypes::CoEObject& object) override;
		void handleErrorStatistic(const datatypes::ErrorStatistic& statistic) override;
		void handleRegister(const datatypes::Register& reg) override;

	private:
		QString tooltip;
	};

} // namespace etherkitten::gui
