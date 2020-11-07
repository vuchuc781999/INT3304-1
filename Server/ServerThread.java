// Ho va ten: Vu Van Chuc
// MSSV: 17020615
// Luong cua server

import java.io.*;
import java.net.*;
import java.nio.ByteBuffer;

public class ServerThread extends Thread {
    public static final int BUFFER_SIZE = 4096;

    private Socket socket;
    private DataInputStream dataInputStream;
    private DataOutputStream dataOutputStream;

    public ServerThread(Socket socket) {
        this.socket = socket;
    }

    public String sendFile(OutputStream outputStream, String fileName) {
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

    public void run() {
        try {
            String message = "";
            boolean getFileName = false;

            dataInputStream = new DataInputStream(socket.getInputStream());
            dataOutputStream = new DataOutputStream(socket.getOutputStream());

            while (true) {
                message = dataInputStream.readUTF();
                System.out.println("Client request: " + message);

                if (getFileName) {
                    message = this.sendFile(socket.getOutputStream(), message);
                    getFileName = false;
                } else {
                    if (message.equals("quit")) {
                        break;
                    } else if (message.equals("get")) {
                        getFileName = true;
                        message = "@getName";
                    } else {
                        message = "@invalidCommand";
                    }
                }

                dataOutputStream.writeUTF(message);
                dataOutputStream.flush();
                System.out.println("Server response: " + message);
            }

            dataInputStream.close();
            dataOutputStream.close();
            socket.close();

            System.out.println("A client disconnected !!!");
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
