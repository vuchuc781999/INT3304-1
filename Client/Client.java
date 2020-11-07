// Ho va ten: Vu Van Chuc
// MSSV: 17020615
// Client nhan file

import java.io.*;
import java.net.*;
import java.nio.ByteBuffer;
import java.util.Arrays;

public class Client {
    public static final String HOST = "127.0.0.1";
    public static final int PORT = 9000;
    public static final int BUFFER_SIZE = 4096;

    private String host;
    private int port;
    private Socket socket;
    private DataInputStream dataInputStream;
    private DataOutputStream dataOutputStream;
    private BufferedReader bufferedReader;

    public Client(String host, int port) {
        try {
            this.host = HOST;
            this.port = PORT;
            this.bufferedReader = new BufferedReader(new InputStreamReader(System.in));
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void receiveFile(InputStream inputStream) {
        int fileNameLength;
        String fileName;
        long fileSize;
        byte[] buffer = new byte[BUFFER_SIZE];
        File file;
        BufferedOutputStream bufferedOutputStream;

        try {
            inputStream.read(buffer, 0, 4);
            fileNameLength = ByteBuffer.wrap(buffer).getInt();

            if (fileNameLength <= 0) {
                return;
            }

            inputStream.read(buffer, 0, fileNameLength);
            fileName = new String(Arrays.copyOfRange(buffer, 0, fileNameLength));
            System.out.println("Server: \"" + fileName + "\"");

            inputStream.read(buffer, 0, 8);
            fileSize = ByteBuffer.wrap(buffer).getLong();
            System.out.println("Server: " + fileSize + " bytes");

            file = new File(fileName);
            bufferedOutputStream = new BufferedOutputStream(new FileOutputStream(file));

            while (fileSize >= BUFFER_SIZE) {
                inputStream.read(buffer, 0, BUFFER_SIZE);
                bufferedOutputStream.write(buffer, 0, BUFFER_SIZE);
                fileSize -= BUFFER_SIZE;
            }

            inputStream.read(buffer, 0, (int) fileSize);
            bufferedOutputStream.write(buffer, 0, (int) fileSize);

            bufferedOutputStream.close();

        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void connect() {
        try {
            String message = "";
            boolean getFile = false;

            socket = new Socket(this.host, this.port);
            dataInputStream = new DataInputStream(socket.getInputStream());
            dataOutputStream = new DataOutputStream(socket.getOutputStream());

            while (true) {
                System.out.print("Client: ");
                message = bufferedReader.readLine();
                if (message.length() > 0) {
                    dataOutputStream.writeUTF(message);
                    dataOutputStream.flush();

                    if (getFile) {
                        this.receiveFile(socket.getInputStream());

                        getFile = false;
                    } else if (message.equals("quit")) {
                        break;
                    }

                    message = dataInputStream.readUTF();
                    System.out.println("Server: " + message);

                    if (message.equals("@getName")) {
                        getFile = true;
                    }
                }
            }

            dataInputStream.close();
            dataOutputStream.close();
            socket.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void main(String[] args) {
        String host = HOST;
        int port = PORT;
        Client client;

        if (args.length > 1) {
            host = args[0];
            port = Integer.parseInt(args[1]);
        }

        client = new Client(host, port);
        client.connect();
    }
}
