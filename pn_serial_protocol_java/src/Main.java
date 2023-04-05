import com.fazecast.jSerialComm.SerialPort;
import tools.ConsoleColors;
import tools.SerialUART;

import java.util.Scanner;
import java.util.Vector;
//import tools.*;

public class Main {
    static volatile boolean transmitted = true;
    public static void test(SerialUART uart){
        uart.enableCanMode();
//        uart.setCanReceiveCallback(flash::receiveCallback);

        uart.setCanTransmitCallback(status -> {
            if(status != SerialUART.CanTransmitStatus.SUCCESS)
                System.out.println("Transmit data failed");
            transmitted = true;
//            flash.transmittedCallback();
//            System.out.println("Transmitted");
        });
        System.out.println(uart.connect() ? "Connected" : "Connection failed");
//        while(true){
        transmitted = true;
        for(int i=1;i<=(10*1024);i++) {
            while (!transmitted)
                Thread.onSpinWait();
            System.out.println("Sending "+i);
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

    public static void main(String[] args) {

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