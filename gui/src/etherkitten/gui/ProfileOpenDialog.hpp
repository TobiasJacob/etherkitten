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

#include <QComboBox>
#include <QDialog>
#include <QWidget>
#include <string>
#include <vector>

namespace etherkitten::gui
{

	/*!
	 * \brief Allows the user to choose which profile to load.
	 */
	class ProfileOpenDialog : public QDialog
	{
		Q_OBJECT
	public:
		/*!
		 * \brief Create a new ProfileOpenDialog.
		 * \param parent The parent widget/window.
		 * \param profiles The available profiles.
		 */
		ProfileOpenDialog(QWidget* parent, std::vector<std::string> profiles);

		/*!
		 * \brief Return the chosen profile.
		 * \return the chosen profile.
		 */
		std::string getProfile() const;

	private:
		QComboBox* box;
		std::string profile;
	};

} // namespace etherkitten::gui
