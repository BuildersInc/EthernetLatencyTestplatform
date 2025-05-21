import socket

# Define the server address and port
HOST = ''  # Listen on all available interfaces
PORT = 8080

# Create a UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Bind the socket to the server address and port
sock.bind((HOST, PORT))

print(f"Serving UDP on port {PORT}...")

# Loop to handle incoming messages
while True:
    # Receive data from the client (maximum buffer size is 1024 bytes)
    data, addr = sock.recvfrom(1024)
    
    print(f"Received message: {data.decode()} from {addr}")
    
    # Send a response to the client
    response = "Hello, World!"  # You can customize the response here
    sock.sendto(response.encode(), addr)
