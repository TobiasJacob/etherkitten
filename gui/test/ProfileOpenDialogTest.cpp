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

#include <etherkitten/gui/ProfileOpenDialog.hpp>
#include <QApplication>
#include <iostream>

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);

	etherkitten::gui::ProfileOpenDialog chooser(
	    nullptr, { "profile1", "profile2", "Hi, I'm also a profile!" });
	if (chooser.exec() == QDialog::Accepted)
	{
		std::cout << chooser.getProfile() << std::endl;
	}
}
