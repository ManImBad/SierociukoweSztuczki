import asyncio
import websockets
import json
import socket # <--- NOWY IMPORT

# --- Konfiguracja "mostu" do C++ ---
CPP_HOST = '127.0.0.1' # localhost
CPP_PORT = 9999       # Dowolny wolny port, na którym C++ będzie słuchał
# Tworzymy gniazdo UDP (AF_INET = IPv4, SOCK_DGRAM = UDP)
udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
print(f"Będę wstrzykiwać dane do C++ na adresie udp://{CPP_HOST}:{CPP_PORT}")
# --- Koniec konfiguracji ---


async def handler(websocket, path):
    print(f"Client connected from {websocket.remote_address}")
    try:
        async for message in websocket:
            try:
                data = json.loads(message)
                accel = data.get('accelerometer', {})
                orient = data.get('orientation', {})
                
                ax = accel.get('x') or 0
                ay = accel.get('y') or 0
                az = accel.get('z') or 0
                
                opitch = orient.get('pitch') or 0
                oroll = orient.get('roll') or 0
                oyaw = orient.get('yaw') or 0

                # --- DRUKUJEMY W KONSOLI (tak jak wcześniej) ---
                print(f"  Accelerometer: x={ax:.2f}, y={ay:.2f}, z={az:.2f}")
                print(f"  Orientation: Pitch={opitch:.2f}, Roll={oroll:.2f}, Yaw={oyaw:.2f}")
                print("-" * 20)

                # --- NOWA LINIA: WSTRZYKNIĘCIE DO C++ ---
                # Tworzymy prosty string, który C++ łatwo zrozumie
                # Format: "pitch roll yaw x y z"
                payload = f"{opitch:.2f} {oroll:.2f} {oyaw:.2f} {ax:.2f} {ay:.2f} {az:.2f}"
                
                # Wysyłamy string jako bajty przez gniazdo UDP
                udp_socket.sendto(payload.encode('utf-8'), (CPP_HOST, CPP_PORT))
                # --- KONIEC NOWEJ LINII ---

            except Exception as e_inner:
                print(f"!!! BŁĄD PRZETWARZANIA WIADOMOŚCI: {e_inner}")
                print(f"    Problematic data: {message[:200]}...")
                print("-" * 20)

    except websockets.exceptions.ConnectionClosed:
        print(f"Client {websocket.remote_address} disconnected")
    except Exception as e_outer:
        print(f"!!! KRYTYCZNY BŁĄD SERWERA: {e_outer}")

async def main():
    async with websockets.serve(handler, "0.0.0.0", 8081):
        print("WebSocket server started.")
        print("Listening on ws://0.0.0.0:8081")
        await asyncio.Future()  # Run forever

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("Server stopped.")