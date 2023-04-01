import com.fazecast.jSerialComm.SerialPort;
import tools.ConsoleColors;
import tools.SerialUART;

import java.util.Scanner;
import java.util.Vector;
//import tools.*;

public class Main {
    public static void test(SerialUART uart){
        uart.enableCanMode();
//        uart.setCanReceiveCallback(flash::receiveCallback);
//        uart.setCanTransmitCallback(status -> {
//            flash.transmittedCallback();
////            System.out.println("Transmitted");
//        });
        System.out.println(uart.connect() ? "Connected" : "Connection failed");
//        while(true){
        uart.send(0x10, new byte[]{1,2,3,4,5,6,7,8});
        System.out.println("Retrying.....");
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
        test(uart);
//        System.out.println(uart.connect() ? "Connected" : "Connection failed");
//        System.out.println(uart.disconnect()?"Disconnected":"Disconnect failed");
    }
}