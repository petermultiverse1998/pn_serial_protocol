/*
 * uart.c
 *
 *  Created on: Jan 25, 2023
 *      Author: NIRUJA
 */
#include "uart.h"
#include "stdarg.h"

#define TRANSMIT_BUFF_SIZE 100

static UART_HandleTypeDef *huart_adapter;
static CRC_HandleTypeDef *hcrc_adapter;

static void (*receiveCallback)(uint32_t, uint8_t*, uint16_t);

typedef enum {
	CONSOLE_ERROR, CONSOLE_INFO, CONSOLE_WARNING
} ConsoleStatus;

//For Receiving
static const uint32_t RECEIVE_TIMEOUT = 10000;

//For Sending
const uint32_t TRANSMIT_TIMEOUT = 10000;
static uint8_t send_data[TRANSMIT_BUFF_SIZE];
static uint8_t send_sync_bytes[1];
static uint8_t send_sync_ack[1];

static void console(ConsoleStatus status, const char *func_name,
		const char *msg, ...) {
	//	if(state!=CONSOLE_ERROR)
	//		return;
	//TODO make naked and show all registers
	if (status == CONSOLE_ERROR) {
		printf("uart.c|%s> ERROR :", func_name);
	} else if (status == CONSOLE_INFO) {
		printf("uart.c|%s> INFO : ", func_name);
	} else if (status == CONSOLE_WARNING) {
		printf("uart.c|%s> WARNING : ", func_name);
	} else {
		printf("uart.c|%s: ", func_name);
	}
	va_list args;
	va_start(args, msg);
	vprintf(msg, args);
	va_end(args);
}

/////////////////////////////////////////////////////////////////

static uint8_t rec_ack[8];
/*
 * It will send bytes and wait for acknowledge
 * @param bytes         : bytes to be sent
 * @param bytes_len     : size of bytes
 * @param ack           : ack to be received
 * @param ack_len       : size of ack
 * @param time_out      : timeout in millisecond
 * @param num_of_try    : number of try
 * @return              : 1 for success
 *                      : 0 for failed
 */
static uint8_t sendAndack(uint8_t *bytes, uint16_t bytes_len, uint8_t *ack,
		uint16_t ack_len, uint32_t time_out, uint8_t num_of_try) {
	uint8_t success_check = 0;
	for (int i = 0; i < num_of_try; i++) {
		if (HAL_UART_Transmit(huart_adapter, bytes, bytes_len, time_out)
				!= HAL_OK) {
			continue;
		}
		HAL_UART_Receive(huart_adapter, rec_ack, ack_len, time_out);
		uint8_t check = 1;
		for (int j = 0; j < ack_len; j++) {
			if (rec_ack[j] != ack[j]) {
				check = 0;
				break;
			}
		}
		if (check) {
			success_check = 1;
			break;
		}
	}
	return success_check;
}


/**
 * This is receive thread called each time there is a receive available
 */
static void receiveThread() {
	/* Start */
	uint8_t start_byte;
	if (HAL_UART_Receive(huart_adapter, (uint8_t*) &start_byte, 1,
			HAL_MAX_DELAY) != HAL_OK)
		return;
	if (start_byte != 'S')
		return;
	console(CONSOLE_INFO, __func__, "Start byte %c received\n",
			(char) start_byte);
	uint8_t start_ack = 'O';
	if (HAL_UART_Transmit(huart_adapter, (uint8_t*) &start_ack, 1,
			RECEIVE_TIMEOUT) != HAL_OK) {
		console(CONSOLE_ERROR, __func__, "Start ack %c sending timeout %d ms\n",
				(char) start_ack, RECEIVE_TIMEOUT);
		return;
	}
	console(CONSOLE_INFO, __func__, "Start ack %c sent\n", (char) start_ack);

	/* Can ID */
	uint32_t can_id;
	if (HAL_UART_Receive(huart_adapter, (uint8_t*) (&can_id), 4,
			RECEIVE_TIMEOUT) != HAL_OK) {
		console(CONSOLE_ERROR, __func__, "Can ID received timeout %d ms\n",
				RECEIVE_TIMEOUT);
		return;
	}
	console(CONSOLE_INFO, __func__, "Can ID received :: 0x%x\n", can_id);
	if (HAL_UART_Transmit(huart_adapter, (uint8_t*) (&can_id), 4,
			RECEIVE_TIMEOUT) != HAL_OK) {
		console(CONSOLE_ERROR, __func__,
				"Can ID ack 0x%x sending timeout %d ms\n", can_id,
				RECEIVE_TIMEOUT);
		return;
	}
	console(CONSOLE_INFO, __func__, "CAN ID ack 0x%x sent\n", can_id);

	/*  data length */
	uint16_t len;
	if (HAL_UART_Receive(huart_adapter, (uint8_t*) (&len), 2, RECEIVE_TIMEOUT)
			!= HAL_OK) {
		console(CONSOLE_ERROR, __func__, "Length received timeout %d ms\n",
				RECEIVE_TIMEOUT);
		return;
	}
	len = 8;
	console(CONSOLE_INFO, __func__, "Length received :: %d\n", len);
	uint8_t len_ack = 'O';
	if (HAL_UART_Transmit(huart_adapter, (uint8_t*) &len_ack, 1,
			RECEIVE_TIMEOUT) != HAL_OK) {
		console(CONSOLE_ERROR, __func__,
				"Length ack %c sending timeout %d ms\n", (char) len_ack,
				RECEIVE_TIMEOUT);
		return;
	}
	console(CONSOLE_INFO, __func__, "Length ack %c sent\n", (char) len_ack);

	/* Data */
	uint8_t data[len];
	if (HAL_UART_Receive(huart_adapter, data, len, RECEIVE_TIMEOUT) != HAL_OK) {
		console(CONSOLE_ERROR, __func__, "Data receive timeout %d ms\n",
				RECEIVE_TIMEOUT);
		return;
	}
	console(CONSOLE_INFO, __func__, "Data received\n");
	int loop_limit = len % 4;
	for (int i = 0; i < (4 - loop_limit) && loop_limit > 0; i++)
		data[len++] = 0x00;

	uint32_t crc = HAL_CRC_Calculate(hcrc_adapter, (uint32_t*) data,
			(uint32_t) (len / 4));
	if (HAL_UART_Transmit(huart_adapter, (uint8_t*) &len_ack, 1,
			RECEIVE_TIMEOUT) != HAL_OK) {
		console(CONSOLE_ERROR, __func__, "CRC 0x%x sending timeout %d ms\n",
				crc, RECEIVE_TIMEOUT);
		return;
	}
	console(CONSOLE_INFO, __func__, "CRC 0x%x sent\n", crc);

	/* Endgame */
	uint8_t end_byte;
	if (HAL_UART_Receive(huart_adapter, (uint8_t*) &end_byte, 1, HAL_MAX_DELAY)
			!= HAL_OK) {
		console(CONSOLE_ERROR, __func__, "End byte received timeout %d ms\n",
				RECEIVE_TIMEOUT);
		return;
	}
	if (end_byte != '\0') {
		console(CONSOLE_ERROR, __func__, "End byte received incorrect : %d\n",
				(char) end_byte);
		return;
	}
	console(CONSOLE_INFO, __func__, "End byte %c received\n", (char) end_byte);
	uint8_t end_ack = 'O';
	if (HAL_UART_Transmit(huart_adapter, (uint8_t*) &end_ack, 1,
			RECEIVE_TIMEOUT) != HAL_OK) {
		console(CONSOLE_ERROR, __func__, "End ack %c sending timeout %d ms\n",
				(char) end_ack, RECEIVE_TIMEOUT);
		return;
	}
	console(CONSOLE_INFO, __func__, "End ack %c sent\n", (char) end_ack);

	console(CONSOLE_INFO, __func__, "Data received success\n");
	receiveCallback(can_id, data, len);
	//after this go to the next phase
}


/**
 *This sends CAN message
 *@param id		: CAN ID
 *@param bytes	: Bytes to be sent
 *@param len 	: length of bytes
 *@return       : 1 for successs
 *				: 0 for failed
 */
static uint8_t sendThread(uint32_t id, uint16_t len) {
	uint32_t timeout = TRANSMIT_TIMEOUT;
	uint8_t num_try = 10;

	send_sync_bytes[0] = 'S';
	send_sync_ack[0] = 'O';

	if (!sendAndack(send_sync_bytes, 1, send_sync_ack, 1, timeout, num_try))
		return 0;

	if (!sendAndack((uint8_t*) (&id), 4, (uint8_t*) (&id), 4, timeout, num_try))
		return 0;

	if (!sendAndack((uint8_t*) (&len), 2, send_sync_ack, 1, timeout, num_try))
		return 0;

	uint32_t new_len = len;
	int loop_limit = new_len % 4;
	for (int i = 0; i < (4 - loop_limit) && loop_limit > 0; i++)
		send_data[new_len++] = 0x00;
	uint32_t crc = HAL_CRC_Calculate(hcrc_adapter, (uint32_t*) send_data,
			new_len / 4);
	uint8_t check = sendAndack(send_data, len, (uint8_t*) (&crc), 4, timeout,
			num_try);

	send_sync_bytes[0] = check ? '\0' : -1;
	if (!sendAndack(send_sync_bytes, 1, send_sync_ack, 1, timeout, num_try))
		return 0;

	return check;
}

////////////////////////////////////////////////////////////////////////////
/**
 * This is called at beginning to initiate
 * @param huart	: UART handler
 * @param hcrc	: CRC handler
 * @param huart	: receive callback function
 */
void uart_init(UART_HandleTypeDef *huart, CRC_HandleTypeDef *hcrc,
		void (*receiveCallbackFunc)(uint32_t, uint8_t*, uint16_t)) {
	huart_adapter = huart;
	hcrc_adapter = hcrc;
	receiveCallback = receiveCallbackFunc;
	console(CONSOLE_INFO, __func__, "Initiate success\n");
}

/**
 * This transmits data to PC
 * @param id	: CAN ID
 * @param bytes	: bytes to be send
 * @param len	: Length of data to be sent
 */
int uart_transmit(uint32_t id, uint8_t *bytes, uint16_t len) {
	for (int i = 0; i < len; i++)
		send_data[i] = bytes[i];
	return sendThread(id, len);
}

/**
 * This is uart loop
 *
 */
void uart_loop() {
	receiveThread();
}
