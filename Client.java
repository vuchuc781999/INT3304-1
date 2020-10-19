import java.io.*;
import java.net.*;

public class Client {
    public static final String HOST = "127.0.0.1";
    public static final int PORT = 9000;

    private String host;
    private int port;
    private Socket connection;
    private DataInputStream inputStream;
    private DataOutputStream outputStream;
    private BufferedReader reader;

    public Client(String host, int port) {
        try {
            this.host = host;
            this.port = port;
            this.reader = new BufferedReader(new InputStreamReader(System.in));
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void connect() {
        try {
            String message = "";

            connection = new Socket(this.host, this.port);
            inputStream = new DataInputStream(connection.getInputStream());
            outputStream = new DataOutputStream(connection.getOutputStream());

            while (true) {
                System.out.print("Client: ");
                message = reader.readLine();
                outputStream.writeUTF(message);
                outputStream.flush();

                if (message.equals("QUIT")) {
                    break;
                }

                message = inputStream.readUTF();
                System.out.println("Server: " + message);
            }

            inputStream.close();
            outputStream.close();
            connection.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void main(String[] args) {
        String host;
        int port;
        Client client;

        if (args.length > 1) {
            host = args[0];
            port = Integer.parseInt(args[1]);
        } else {
            host = HOST;
            port = PORT;
        }

        client = new Client(host, port);
        client.connect();
    }
}
