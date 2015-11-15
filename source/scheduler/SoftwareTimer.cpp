/**
 * \file
 * \brief SoftwareTimer class implementation
 *
 * \author Copyright (C) 2015 Kamil Szczygiel http://www.distortec.com http://www.freddiechopin.info
 *
 * \par License
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \date 2015-11-15
 */

#include "distortos/SoftwareTimer.hpp"

namespace distortos
{

/*---------------------------------------------------------------------------------------------------------------------+
| public functions
+---------------------------------------------------------------------------------------------------------------------*/

SoftwareTimer::~SoftwareTimer()
{

}

int SoftwareTimer::start(const TickClock::duration duration)
{
	const auto now = TickClock::now();
	return start(now + duration + decltype(duration){1});
}

/*---------------------------------------------------------------------------------------------------------------------+
| protected static functions
+---------------------------------------------------------------------------------------------------------------------*/

void SoftwareTimer::softwareTimerRunner(SoftwareTimer& softwareTimer)
{
	softwareTimer.run();
}

}	// namespace distortos
