#pragma once
#ifndef __LOGGER_HARDWARE_H__
#define __LOGGER_HARDWARE_H__

#include "logger.hpp"
#include "stdint.h"
#include "main.h"
#include "init.h"

constexpr	uint16_t I2C_EEPROM_ADDRESS = 0x57;
constexpr	uint16_t MAX_EEPROM_ADDRESS = 0x1fff;
constexpr	uint32_t I2C_EEPROM_TIMEOUT = 0x10;
I2C_HandleTypeDef *phi2c = &hi2c1;


class MemoryLoggerHardware : public IMemoryLogger
{
	public:
		virtual int8_t MemoryWrite (uint32_t Address, uint8_t *Data, uint32_t Size) override;
		virtual int8_t MemoryRead (uint32_t Address, uint8_t *Data, uint32_t Size) override;
		virtual void TransmitLogToOtherHardware (uint8_t *Data, uint16_t Length) override;
		virtual uint32_t GetMaxAddress () override;
};

//I2C bugfix for stm32f103:
static void I2C_ClearBusyFlagErratum (I2C_HandleTypeDef *hi2c, uint32_t timeout);
static uint8_t wait_for_gpio_state_timeout (GPIO_TypeDef *port, uint16_t pin, GPIO_PinState state, uint32_t timeout);

int8_t MemoryLoggerHardware::MemoryWrite (uint32_t Address, uint8_t *Data, uint32_t Size)
{
	if ( HAL_OK != HAL_I2C_Mem_Write(phi2c, I2C_EEPROM_ADDRESS << 1, static_cast<uint16_t>(Address), I2C_MEMADD_SIZE_16BIT, Data, static_cast<uint16_t>(Size), I2C_EEPROM_TIMEOUT) )
	{
		I2C_ClearBusyFlagErratum (phi2c, 1000);
		return -1;
	}
	return 1;
}

int8_t MemoryLoggerHardware::MemoryRead (uint32_t Address, uint8_t *Data, uint32_t Size)
{
	if ( HAL_OK != HAL_I2C_Mem_Read (phi2c, I2C_EEPROM_ADDRESS << 1, static_cast<uint16_t>(Address), I2C_MEMADD_SIZE_16BIT, Data, static_cast<uint16_t>(Size), I2C_EEPROM_TIMEOUT))
	{
		//lambda ("Can't read settings from EEPROM! \n\r");
		return -1;
	}
	return 1;
}

void MemoryLoggerHardware::TransmitLogToOtherHardware (uint8_t *Data, uint16_t Length)
{
	HAL_GPIO_WritePin (RS485_DIR_GPIO_Port, RS485_DIR_Pin, GPIO_PIN_SET);
	HAL_UART_Transmit (&huart3, Data, Length, 100);
	HAL_GPIO_WritePin (RS485_DIR_GPIO_Port, RS485_DIR_Pin, GPIO_PIN_RESET);
}

uint32_t MemoryLoggerHardware::GetMaxAddress ()
{
	return static_cast<uint32_t>(MAX_EEPROM_ADDRESS);
}

static void I2C_ClearBusyFlagErratum (I2C_HandleTypeDef *hi2c, uint32_t timeout)
{
	// 2.13.7 I2C analog filter may provide wrong value, locking BUSY. STM32F10xx8 STM32F10xxB Errata sheet
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    // 1. Clear PE bit.
    CLEAR_BIT(hi2c->Instance->CR1, I2C_CR1_PE);

    //  2. Configure the SCL and SDA I/Os as General Purpose Output Open-Drain, High level (Write 1 to GPIOx_ODR).
    HAL_I2C_DeInit(hi2c);

    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStructure.Pull = GPIO_NOPULL;

    GPIO_InitStructure.Pin = GPIO_PIN_6; // SCL
    HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = GPIO_PIN_7; // SDA
    HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 3. Check SCL and SDA High level in GPIOx_IDR.
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);

    wait_for_gpio_state_timeout(GPIOB, GPIO_PIN_6, GPIO_PIN_SET, timeout);
    wait_for_gpio_state_timeout(GPIOB, GPIO_PIN_7, GPIO_PIN_SET, timeout);

    // 4. Configure the SDA I/O as General Purpose Output Open-Drain, Low level (Write 0 to GPIOx_ODR).
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);

    // 5. Check SDA Low level in GPIOx_IDR.
    wait_for_gpio_state_timeout(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET, timeout);

    // 6. Configure the SCL I/O as General Purpose Output Open-Drain, Low level (Write 0 to GPIOx_ODR).
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);

    // 7. Check SCL Low level in GPIOx_IDR.
    wait_for_gpio_state_timeout(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET, timeout);

    // 8. Configure the SCL I/O as General Purpose Output Open-Drain, High level (Write 1 to GPIOx_ODR).
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);

    // 9. Check SCL High level in GPIOx_IDR.
    wait_for_gpio_state_timeout(GPIOB, GPIO_PIN_6, GPIO_PIN_SET, timeout);

    // 10. Configure the SDA I/O as General Purpose Output Open-Drain , High level (Write 1 to GPIOx_ODR).
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);

    // 11. Check SDA High level in GPIOx_IDR.
    wait_for_gpio_state_timeout(GPIOB, GPIO_PIN_7, GPIO_PIN_SET, timeout);

    // 12. Configure the SCL and SDA I/Os as Alternate function Open-Drain.
    GPIO_InitStructure.Mode = GPIO_MODE_AF_OD;
    //GPIO_InitStructure.Alternate = GPIO_AF4_I2C2; // F4

    GPIO_InitStructure.Pin = GPIO_PIN_6;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = GPIO_PIN_7;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 13. Set SWRST bit in I2Cx_CR1 register.
    SET_BIT(hi2c->Instance->CR1, I2C_CR1_SWRST);
    __ASM volatile ("NOP");

    /* 14. Clear SWRST bit in I2Cx_CR1 register. */
    CLEAR_BIT(hi2c->Instance->CR1, I2C_CR1_SWRST);
    __ASM volatile ("NOP");

    /* 15. Enable the I2C peripheral by setting the PE bit in I2Cx_CR1 register */
    SET_BIT(hi2c->Instance->CR1, I2C_CR1_PE);
    __ASM volatile ("NOP");

    // Call initialization function.
    HAL_I2C_Init(hi2c);
}

static uint8_t wait_for_gpio_state_timeout (GPIO_TypeDef *port, uint16_t pin, GPIO_PinState state, uint32_t timeout)
 {
    uint32_t Tickstart = HAL_GetTick();
    uint8_t ret = 1;

    for(;(state != HAL_GPIO_ReadPin(port, pin)) && (1 == ret);) // Wait until flag is set
    {
        if(timeout != HAL_MAX_DELAY) // Check for the timeout
        {
            if((timeout == 0U) || ((HAL_GetTick() - Tickstart) > timeout)) ret = 0;
        }

        __ASM volatile ("NOP");
    }
    return ret;
}

#endif
