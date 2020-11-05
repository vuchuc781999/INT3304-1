
/**
 * Ho va ten: Vu Van Chuc
 * MSSV: 17020615
 * Server gui file
*/

import java.io.*;
import java.net.*;
import java.nio.ByteBuffer;

public class Server {
    public static final int PORT = 9000;
    public static final int BUFFER_SIZE = 4096;

    private int port;
    private ServerSocket serverSocket;
    private Socket socket;
    private DataInputStream dataInputStream;
    private DataOutputStream dataOutputStream;

    public Server(int port) {
        try {
            this.port = port;

            this.serverSocket = new ServerSocket(this.port);
            this.serverSocket.setReuseAddress(true);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void listen() {
        try {
            String message = "";
            boolean getFileName = false;

            while (true) {
                System.out.println("Waiting for a client...");
                socket = serverSocket.accept();
                System.out.println("A client connected !!!");
                dataInputStream = new DataInputStream(socket.getInputStream());
                dataOutputStream = new DataOutputStream(socket.getOutputStream());

                while (true) {
                    message = dataInputStream.readUTF();
                    System.out.println("Client request: " + message);

                    if (message.equals("quit")) {
                        break;
                    }

                    if (getFileName) {
                        message = this.sendFile(socket.getOutputStream(), message);
                        getFileName = false;
                    } else {
                        if (message.equals("get")) {
                            getFileName = true;
                            message = "@getName";
                        } else {
                            message = "@invalidCommand";
                        }
                    }

                    System.out.println("Server response: " + message);
                    dataOutputStream.writeUTF(message);
                    dataOutputStream.flush();
                }

                dataInputStream.close();
                dataOutputStream.close();
                socket.close();
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    protected String sendFile(OutputStream outputStream, String fileName) {
        String message = "@ok";
        long fileSize;
        byte[] buffer;

        try {
            File file = new File(fileName);

            if (file.exists()) {
                BufferedInputStream bufferedInputStream = new BufferedInputStream(new FileInputStream(file));
                fileSize = file.length();

                buffer = ByteBuffer.allocate(4).putInt(fileName.length()).array();
                outputStream.write(buffer, 0, buffer.length);

                buffer = fileName.getBytes();
                outputStream.write(buffer, 0, buffer.length);

                buffer = ByteBuffer.allocate(8).putLong(fileSize).array();
                outputStream.write(buffer, 0, buffer.length);

                buffer = new byte[BUFFER_SIZE];

                while (fileSize >= BUFFER_SIZE) {
                    bufferedInputStream.read(buffer, 0, BUFFER_SIZE);
                    outputStream.write(buffer, 0, BUFFER_SIZE);
                    fileSize -= BUFFER_SIZE;
                }

                bufferedInputStream.read(buffer, 0, (int) fileSize);
                outputStream.write(buffer, 0, (int) fileSize);

                bufferedInputStream.close();
            } else {
                buffer = ByteBuffer.allocate(4).putInt(0).array();
                outputStream.write(buffer, 0, buffer.length);
                message = "@notFound";
            }
        } catch (Exception e) {
            e.printStackTrace();
        }

        return message;
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
