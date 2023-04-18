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