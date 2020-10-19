import java.io.*;
import java.net.*;

public class Server {
    public static final int PORT = 9000;

    private int port;
    private ServerSocket server;
    private Socket connection;
    private DataInputStream inputStream;
    private DataOutputStream outputStream;

    public Server(int port) {
        try {
            this.port = port;
            this.server = new ServerSocket(this.port);
            this.server.setReuseAddress(true);
        } catch (Exception e) {
            e.printStackTrace();
        }

    }

    public void listen() {
        try {
            String message = "";
            boolean hello = false;
            boolean getInfo = false;

            while (true) {
                System.out.println("Waiting for a client...");
                connection = server.accept();
                inputStream = new DataInputStream(connection.getInputStream());
                outputStream = new DataOutputStream(connection.getOutputStream());

                while (true) {
                    message = inputStream.readUTF();
                    System.out.println("Client request: " + message);

                    if (message.equals("QUIT")) {
                        break;
                    }

                    if (hello) {
                        if (getInfo) {
                            try {

                            } catch (Exception e) {
                                message = "Wrong Format Data";
                                e.printStackTrace();
                            }

                            getInfo = false;
                        } else {
                            if (message.equals("USER INFO")) {
                                message = "210 OK";
                                getInfo = true;
                            } else {
                                message = "Command Not Found";
                            }
                        }
                    } else {
                        if (message.equals("HELO Server")) {
                            message = "200 Hello Client";
                            hello = true;
                        } else {
                            message = "Command Not Found";
                        }
                    }

                    System.out.println("Server response: " + message);
                    outputStream.writeUTF(message);
                    outputStream.flush();
                }

                hello = false;

                inputStream.close();
                outputStream.close();
                connection.close();
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void main(String[] args) {
        int port;
        Server server;

        if (args.length > 0) {
            port = Integer.parseInt(args[0]);
        } else {
            port = PORT;
        }

        server = new Server(port);
        server.listen();
    }
}
