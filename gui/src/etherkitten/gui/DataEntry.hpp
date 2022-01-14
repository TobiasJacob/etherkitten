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

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpacerItem>
#include <QString>
#include <QWidget>
#include <string>

namespace etherkitten::gui
{

	/*!
	 * \brief Represents a special entry in the WatchList or SlaveTree.
	 */
	class DataEntry : public QWidget
	{
		Q_OBJECT

	public:
		/*!
		 * \brief Return whether the entry is being edited or not.
		 * This is used to tell the WatchList or SlaveTree whether it
		 * should automatically update the displayed value.
		 * \return Whether the entry is being edited or not.
		 */
		virtual bool editing() const = 0;

		/*!
		 * \brief Set whether the widget is being edited or not,
		 * and update the the appearance of the widget accordingly.
		 * \param edit Whether the widget is being edited or not.
		 */
		virtual void setEditing(bool edit) = 0;

		/*!
		 * \brief Set the displayed value.
		 * \param value The new value.
		 */
		virtual void setValue(std::string value) = 0;

		/*!
		 * \brief Set the data entry to active or inactive.
		 * When it is inactive, the write button is disabled, if it exists.
		 */
		virtual void setActive(bool active) = 0;

		/*!
		 * \brief Get the currently displayed value.
		 * \return The current value.
		 */
		virtual std::string getValue() const = 0;
	signals:
		/*!
		 * Request that the current value be written to the appropriate slave.
		 */
		void requestWrite();
	};

	class EditableDataEntry : public DataEntry
	{
		Q_OBJECT
	public:
		EditableDataEntry();
		void setEditing(bool edit) override;
		bool editing() const override;
		void setValue(std::string value) override;
		std::string getValue() const override;
		void setActive(bool active) override;

	private:
		QPushButton* writeButton;
		QPushButton* resetButton;
		QLineEdit* textEdit;
		QHBoxLayout* layout;
		QSpacerItem* spacer;
		QLabel* editLabel;
		QString oldText;
		bool isEditing;
	};

} // namespace etherkitten::gui
