# pn_serial_protocol
This protocol is for UART communication using DMA in receive.   
Pros of this communication protocol are:  
→It is faster and more reliable  
→In case of error it retries and recovers the communication  
→It uses call back  

# main.java  
```rb
import com.fazecast.jSerialComm.SerialPort;
import tools.ConsoleColors;
import tools.SerialUART;

import java.util.Arrays;
import java.util.Scanner;
import java.util.Vector;
//import tools.*;

public class Main {
    static volatile boolean transmitted = true;
    public static void test(SerialUART uart){
        uart.enableCanMode();
//        uart.setCanReceiveCallback(flash::receiveCallback);
        uart.setCanReceiveCallback((id, bytes) -> {
            System.out.printf("0x%x (%d)\n",id,bytes.length);
            transmitted = true;
//            StringBuilder builder = new StringBuilder();
//            for(byte b: bytes)
//                builder.append((char) b);
//            System.out.println(builder);
        });

        uart.setCanTransmitCallback(status -> {
            if(status != SerialUART.CanTransmitStatus.SUCCESS)
                System.out.println("Transmit data failed");
//            transmitted = true;
//            flash.transmittedCallback();
//            System.out.println("Transmitted");
        });
        System.out.println(uart.connect() ? "Connected" : "Connection failed");
//        while(true){
        transmitted = true;
        for(int i=1;i<=(10);i++) {
            while (!transmitted)
                Thread.onSpinWait();
//            try { 
//                Thread.sleep(100);
//            } catch (InterruptedException e) {
//                throw new RuntimeException(e);
//            }
//            System.out.println("Sending "+i);
            uart.send(i, new byte[]{1, 2, 3, 4, 5, 6, 7, 8});
            transmitted = false;
//            try {
//                Thread.sleep(10);
//            } catch (InterruptedException e) {
//                throw new RuntimeException(e);
//            }
        }
//        System.out.println("Retrying.....");
//        }
    }

    static void print(){
        StackTraceElement element = new Exception().getStackTrace()[1];
        String msg = "There is error";
        System.out.println(ConsoleColors.RED+element+":"+msg+ConsoleColors.RESET);
//        System.out.println("("+element.getFileName()+":"+element.getLineNumber()+")");
    }

    public static void main(String[] args) {
//        print();
//        if(true)
//            return;

        SerialPort[] ports = SerialPort.getCommPorts();
        if (ports == null)
            return;
        if (ports.length == 0)
            return;
        SerialPort port = null;
        for (SerialPort p : ports) {
            System.out.println(p.getDescriptivePortName());
            if(p.getDescriptivePortName().contains("COM4"))
                port = p;
        }

        if(port==null) {
            System.out.println("NULL PORT");
            return;
        }

        SerialUART uart = new SerialUART(port, 115200);
        long tic = System.currentTimeMillis();
        test(uart);
        System.out.println((System.currentTimeMillis()-tic)+" ms");
//        System.out.println(uart.connect() ? "Connected" : "Connection failed");
//        System.out.println(uart.disconnect()?"Disconnected":"Disconnect failed");
    }
}
```

# main.c
```rb
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
#define SEND_TIME_OUT 1000

extern UART_HandleTypeDef huart1;
extern CRC_HandleTypeDef hcrc;

static void console(const char *title, const char *msg) {
	printf("%s:: %s\n", title, msg);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	uart_receiveRxCpltCallback();
}

void receive(uint32_t id, uint8_t *bytes, uint16_t len) {
	printf("0x%x : (%d)\n", (int) id, len);
	uart_transmit(id, bytes, len);
	//	can_com_addMessage_tx(LINK_ID, id, bytes, len, 1);
//	uint32_t prev_tick = HAL_GetTick();
//	int count = 0;
//	uint16_t new_size = 0;

//	while (1) {
//		if (HAL_GetTick() - prev_tick > SEND_TIME_OUT)
//			break;
//		new_size = (len - count);
//		if (new_size > 8)
//			new_size = 8;
//		if (new_size <= 0)
//			break;
//		if(!canTransmitted)
//			continue;
////		printf("%d\n",count);
//		if (canSend(id, bytes + count, new_size)) {
//			prev_tick = HAL_GetTick();
//			count += new_size;
//			canTransmitted = 0;
//		}
//	}
}

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

uint32_t id = 0;
/**
 * This function is called repeatedly
 */
void loop() {
//	id++;

//	static uint32_t prevTick = 0;
//	baseLayerLoop();
//	mapLoop();
//	if ((HAL_GetTick() - prevTick) >= 10) {
//		printf("0x%02x\n",(int)id);
//		can_com_addMessage_tx(LINK_ID, id++, data2, 100, 1);
//		prevTick = HAL_GetTick();
//	}
//	if (id < 10)
//		uart_transmit(id, data1, 5);

	uart_loop();

//	HAL_Delay(1);
}
```

# Output of JAVA
```rb
Silicon Labs CP210x USB to UART Bridge (COM4)
Connected
0x1 (8)
0x2 (8)
0x3 (8)
0x4 (8)
0x5 (8)
0x6 (8)
0x7 (8)
0x8 (8)
0x9 (8)
219 ms
```

# Output of STM32
```rb
INIT FROM ADAPTER:: SUCCESS
0x1 : (8)
0x2 : (8)
0x3 : (8)
0x4 : (8)
0x5 : (8)
0x6 : (8)
0x7 : (8)
0x8 : (8)
0x9 : (8)
0xa : (8)
```
