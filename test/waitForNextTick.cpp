/**
 * \file
 * \brief waitForNextTick() implementation
 *
 * \author Copyright (C) 2014-2015 Kamil Szczygiel http://www.distortec.com http://www.freddiechopin.info
 *
 * \par License
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "waitForNextTick.hpp"

#include "distortos/ThisThread.hpp"

namespace distortos
{

namespace test
{

/*---------------------------------------------------------------------------------------------------------------------+
| global functions
+---------------------------------------------------------------------------------------------------------------------*/

void waitForNextTick()
{
	ThisThread::sleepFor(TickClock::duration{1});
}

}	// namespace test

}	// namespace distortos
