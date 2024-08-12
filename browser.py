import socket

def check_connection(host, port):
    try:
        # Create a socket object
        s = socket.create_connection((host, port), timeout=5)
        s.close()  # Close the connection
        return True
    except socket.error as e:
        print(f"Failed to connect to {host} on port {port}. Error: {e}")
        return False

host = "ns.safaricom.pro"
port = 53

if check_connection(host, port):
    print(f"Successfully connected to {host} on port {port}")
else:
    print(f"Failed to connect to {host} on port {port}")


nslookup ns.safaricom.pro 10.0.0.1
