/**
 * \file
 * \brief SpiMasterLowLevelInterruptBased class implementation for SPIv2 in STM32
 *
 * \author Copyright (C) 2016-2018 Kamil Szczygiel http://www.distortec.com http://www.freddiechopin.info
 *
 * \par License
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "distortos/chip/SpiMasterLowLevelInterruptBased.hpp"

#include "distortos/chip/STM32-SPIv2.hpp"
#include "distortos/chip/STM32-SPIv2-SpiPeripheral.hpp"

#include "distortos/devices/communication/SpiMasterBase.hpp"

#include "distortos/assert.h"

#include <cerrno>

namespace distortos
{

namespace chip
{

/*---------------------------------------------------------------------------------------------------------------------+
| public functions
+---------------------------------------------------------------------------------------------------------------------*/

SpiMasterLowLevelInterruptBased::~SpiMasterLowLevelInterruptBased()
{
	if (isStarted() == false)
		return;

	// reset peripheral
	spiPeripheral_.writeCr1({});
	spiPeripheral_.writeCr2({});
}

std::pair<int, uint32_t> SpiMasterLowLevelInterruptBased::configure(const devices::SpiMode mode,
		const uint32_t clockFrequency, const uint8_t wordLength, const bool lsbFirst, const uint32_t dummyData)
{
	if (isStarted() == false)
		return {EBADF, {}};

	if (isTransferInProgress() == true)
		return {EBUSY, {}};

	const auto ret = configureSpi(spiPeripheral_, mode, clockFrequency, wordLength, lsbFirst);
	if (ret.first != 0)
		return ret;

	dummyData_ = dummyData;
	wordLength_ = wordLength;
	return ret;
}

void SpiMasterLowLevelInterruptBased::interruptHandler()
{
	const uint16_t word = spiPeripheral_.readDr(wordLength_);
	const auto readBuffer = readBuffer_;
	auto readPosition = readPosition_;
	if (readBuffer != nullptr)
	{
		readBuffer[readPosition++] = word;
		if (wordLength_ > 8)
			readBuffer[readPosition++] = word >> 8;
	}
	else
		readPosition += (wordLength_ + 8 - 1) / 8;
	readPosition_ = readPosition;

	if (readPosition < size_)	// transfer not finished yet?
	{
		writeNextItem();
		return;
	}

	// transfer finished
	spiPeripheral_.writeCr2(spiPeripheral_.readCr2() & ~SPI_CR2_RXNEIE);	// disable RXNE interrupt

	const auto spiMasterBase = spiMasterBase_;

	spiMasterBase_ = {};
	readBuffer_ = {};
	writeBuffer_ = {};
	size_ = {};
	readPosition_ = {};
	writePosition_ = {};

	assert(spiMasterBase != nullptr);
	spiMasterBase->transferCompleteEvent(readPosition);
}

int SpiMasterLowLevelInterruptBased::start()
{
	if (isStarted() == true)
		return EBADF;

	wordLength_ = 8;
	spiPeripheral_.writeCr1(SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_SPE | SPI_CR1_BR | SPI_CR1_MSTR);
	spiPeripheral_.writeCr2(SPI_CR2_FRXTH | (wordLength_ - 1) << SPI_CR2_DS_Pos);
	started_ = true;

	return 0;
}

int SpiMasterLowLevelInterruptBased::startTransfer(devices::SpiMasterBase& spiMasterBase, const void* const writeBuffer,
		void* const readBuffer, const size_t size)
{
	if (size == 0)
		return EINVAL;

	if (isStarted() == false)
		return EBADF;

	if (isTransferInProgress() == true)
		return EBUSY;

	if (size % ((wordLength_ + 8 - 1) / 8) != 0)
		return EINVAL;

	spiMasterBase_ = &spiMasterBase;
	readBuffer_ = static_cast<uint8_t*>(readBuffer);
	writeBuffer_ = static_cast<const uint8_t*>(writeBuffer);
	size_ = size;
	readPosition_ = 0;
	writePosition_ = 0;

	spiPeripheral_.writeCr2(spiPeripheral_.readCr2() | SPI_CR2_RXNEIE);	// enable RXNE interrupt
	writeNextItem();	// write first item to start the transfer

	return 0;
}

int SpiMasterLowLevelInterruptBased::stop()
{
	if (isStarted() == false)
		return EBADF;

	if (isTransferInProgress() == true)
		return EBUSY;

	// reset peripheral
	spiPeripheral_.writeCr1({});
	spiPeripheral_.writeCr2({});
	started_ = false;
	return 0;
}

/*---------------------------------------------------------------------------------------------------------------------+
| private functions
+---------------------------------------------------------------------------------------------------------------------*/

void SpiMasterLowLevelInterruptBased::writeNextItem()
{
	const auto writeBuffer = writeBuffer_;
	auto writePosition = writePosition_;
	uint16_t word;
	if (writeBuffer != nullptr)
	{
		const uint16_t characterLow = writeBuffer[writePosition++];
		const uint16_t characterHigh = wordLength_ > 8 ? writeBuffer[writePosition++] : 0;
		word = characterLow | characterHigh << 8;
	}
	else
	{
		writePosition += (wordLength_ + 8 - 1) / 8;
		word = dummyData_;
	}
	writePosition_ = writePosition;
	spiPeripheral_.writeDr(wordLength_, word);
}

}	// namespace chip

}	// namespace distortos