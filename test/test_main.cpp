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

#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

#include <string>

#include "test_globals.hpp"

int etherCATSocket = 0; // NOLINT

int main(int argc, char* argv[])
{
	int result = 0;
	// Additional param to account for running under raw_socket.
	if (argc >= 4 && std::string(argv[3]) == "--raw-socket-mode") // NOLINT
	{
		// raw_socket passes the socket via the first command line argument
		etherCATSocket = std::stoi(argv[1]); // NOLINT

		// Move program name to the index where catch finds it
		argv[3] = argv[1];
		// Remove the arguments catch doesn't know how to deal with
		result = Catch::Session().run(argc - 3, argv + 3);
	}
	else
	{
		result = Catch::Session().run(argc, argv);
	}

	return result;
}
