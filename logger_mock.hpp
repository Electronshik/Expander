#pragma once
#include "../Inc/logger.hpp"
#include "stdint.h"
#include <cstring>

constexpr	uint16_t MAX_EEPROM_ADDRESS = 1023;

class TestLoggerHardware : public IMemoryLogger
{
	public:
		virtual int8_t MemoryWrite(uint32_t Address, uint8_t* Data, uint32_t Size) override;
		virtual int8_t MemoryRead(uint32_t Address, uint8_t* Data, uint32_t Size) override;
		virtual void TransmitLogToOtherHardware(uint8_t* Data, uint16_t Length) override;
		virtual uint32_t GetMaxAddress() override;
		uint8_t TestBuffer[MAX_EEPROM_ADDRESS + 1];
		uint8_t TestOtherInterface[256];
};

int8_t TestLoggerHardware::MemoryWrite(uint32_t Address, uint8_t* Data, uint32_t Size)
{
	uint32_t i = Address;
	for (uint32_t j = 0; j < Size; j++)
	{
		if (i >= MAX_EEPROM_ADDRESS)
		{
			i = 4;
		}
		TestBuffer[i] = Data[j];
		i++;
	}
	return 1;
}

int8_t TestLoggerHardware::MemoryRead(uint32_t Address, uint8_t* Data, uint32_t Size)
{
	uint32_t i = Address;
	for (uint32_t j = 0; j < Size; j++)
	{
		if (i > MAX_EEPROM_ADDRESS)
		{
			i = 4;
		}
		Data[j] = TestBuffer[i];
		i++;
	}
	return 1;
}

void TestLoggerHardware::TransmitLogToOtherHardware(uint8_t* Data, uint16_t Length)
{
	//std::cout << "data" << std::endl;
	for (int i = 0; i < Length; i++)
	{
		TestOtherInterface[i] = Data[i];
	}
}

uint32_t TestLoggerHardware::GetMaxAddress()
{
	return static_cast<uint32_t>(MAX_EEPROM_ADDRESS);
}


