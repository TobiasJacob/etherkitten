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

#include <QCheckBox>
#include <QDialog>
#include <QSpinBox>
#include <QWidget>
#include <chrono>

namespace etherkitten::gui
{

	/*!
	 * \brief Allows the user to specify how far in the past the log should begin when saving it.
	 */
	class LogStartDialog : public QDialog
	{
		Q_OBJECT

	public:
		/*!
		 * \brief Create a new LogStartDialog.
		 * \param parent The parent widget/window.
		 */
		LogStartDialog(QWidget* parent);

		/*!
		 * \brief Return the number of seconds in the past that the
		 * log should start. If all data should be saved, std::chrono::seconds::max()
		 * is returned.
		 * \return the number of seconds in the past that the log should start.
		 */
		std::chrono::seconds getOffset() const;

	private:
		std::chrono::seconds totalSeconds;
		QSpinBox* seconds;
		QSpinBox* minutes;
		QSpinBox* hours;
		QCheckBox* checkbox;
	};

} // namespace etherkitten::gui
