// Ho va ten: Vu Van Chuc
// MSSV: 17020615
// Server gui file

import java.io.*;
import java.net.*;

public class Server {
    public static final int PORT = 9000;

    private int port;
    private ServerSocket serverSocket;
    private Socket socket;

    public Server(int port) {
        try {
            this.port = port;

            this.serverSocket = new ServerSocket(this.port);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void listen() {
        ServerThread serverThread;

        System.out.println("Wait for a client ...");

        try {
            while (true) {
                socket = serverSocket.accept();
                System.out.println("A client connected !!!");

                serverThread = new ServerThread(socket);
                serverThread.start();
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void main(String[] args) {
        int port = PORT;
        Server server;

        if (args.length > 0) {
            port = Integer.parseInt(args[0]);
        }

        server = new Server(port);
        server.listen();
    }
}
