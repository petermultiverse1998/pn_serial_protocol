/*
 * adapter.c
 *
 *  Created on: Dec 29, 2022
 *      Author: NIRUJA GHIMIRE
 */

#include "adapter.h"
#include "string.h"
#include "stdlib.h"
#include "uart.h"
#define SEND_TIME_OUT 10000

extern UART_HandleTypeDef huart1;
extern CRC_HandleTypeDef hcrc;

void console(const char *title, const char *msg) {
	printf("%s:: %s\n", title, msg);
}

static volatile uint8_t canTransmitted = 1;
void receive(uint32_t id, uint8_t *bytes, uint16_t len) {
	printf("0x%x : (%d)\n", (int) id, len);
	//	can_com_addMessage_tx(LINK_ID, id, bytes, len, 1);
	uint32_t prev_tick = HAL_GetTick();
	int count = 0;
	uint16_t new_size = 0;
	while (1) {
		if (HAL_GetTick() - prev_tick > SEND_TIME_OUT)
			break;
		new_size = (len - count);
		if (new_size > 8)
			new_size = 8;
		if (new_size <= 0)
			break;
		if(!canTransmitted)
			continue;
		printf("%d\n",count);
//		if (canSend(id, bytes + count, new_size)) {
			prev_tick = HAL_GetTick();
			count += new_size;
			canTransmitted = 0;
//		}
	}
}

/////////////////////////////////////MAP LAYER///////////////////////////////////////////
//static void can_receive(uint32_t id, uint8_t *bytes, uint8_t len) {
//
////	printf("0x%02x : ", (unsigned int) id);
////	for (int i = 0; i < len; ++i)
////		printf("%d ", bytes[i]);
////	printf("\n");
//
//	//Transmitter
//	can_com_receive_tx(LINK_ID, id, bytes, len);
//
//	//Receiver
//	can_com_receive_rx(LINK_ID, id, bytes, len);
//
//}
//static void dataReceivedCallback(uint32_t link_id, uint32_t id, uint16_t size) {
//	printf("Received-> 0x%02x : ", (unsigned int) id);
//	uint8_t bytes[size];
//	can_com_getBytes_rx(link_id, id, bytes);
//	for (int i = 0; i < size; ++i)
//		printf("%d ", bytes[i]);
//	printf("\n");
////	uart_transmit(id,bytes, size);
//}
//static void mapInit() {
//	//Initiate transmit and receive
//	can_com_init_tx(HAL_GetTick, base_layer_can_send, printf, 0);
//	can_com_addLink_tx(LINK_ID, &link);
//
//	can_com_init_rx(HAL_GetTick, base_layer_can_send, dataReceivedCallback,
//			printf);
//	can_com_addLink_rx(LINK_ID, &link);
//}
//static void mapLoop() {
//	//monitoring
//	can_com_timeoutMonitor_tx(LINK_ID);
//	can_com_errorHandler_tx(LINK_ID);
//	can_com_timeoutMonitor_rx(LINK_ID);
//}

/////////////////////////////////////BASE LAYER///////////////////////////////////////////
//static void baseLayerInit() {
//	base_layer_can_init(canSend, can_receive, printf);
//}
//static void baseLayerLoop() {
//	base_layer_can_send_monitor();
//	base_layer_can_receive_monitor();
//}

////////////////////////////////////MAIN CODE///////////////////////////////////////////
uint8_t data1[] = { 1, 2, 3, 4, 5 };
uint8_t data2[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, [99] = 12 };
/*
 * This function is called at the beginning of our code
 * @param adapter_huart		: handler of UART used for receiving and transmitting data
 */
void init() {
	uart_init(&huart1, &hcrc, receive);
	console("INIT FROM ADAPTER", "SUCCESS");
}

/**
 * This function is called repeatedly
 */
void loop() {
	uart_loop();
}

