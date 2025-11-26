import asyncio
import websockets
import json
import socket

# --- Konfiguracja "mostu" do C++ ---
CPP_HOST = '127.0.0.1' # localhost
CPP_PORT = 9999       # Port nasłuchujący w aplikacji C++
udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
print(f"Będę wstrzykiwać SUROWE dane do C++ na adresie udp://{CPP_HOST}:{CPP_PORT}")
# --- Koniec konfiguracji ---

async def handler(websocket, path):
    print(f"Client connected from {websocket.remote_address}")
    try:
        async for message in websocket:
            try:
                data = json.loads(message)
                
                # 1. Pobieramy sekcje z JSON-a
                accel = data.get('accelerometer', {})
                gyro  = data.get('gyroscope', {})
                mag   = data.get('magnetometer', {})
                ts    = data.get('timestamp', 0) # Timestamp (opcjonalny, ale przydatny do fuzji)

                # 2. Pobieramy wartości (x, y, z) - domyślnie 0.0 jeśli brak
                ax, ay, az = accel.get('x', 0), accel.get('y', 0), accel.get('z', 0)
                gx, gy, gz = gyro.get('x', 0),  gyro.get('y', 0),  gyro.get('z', 0)
                mx, my, mz = mag.get('x', 0),   mag.get('y', 0),   mag.get('z', 0)

                # --- DRUKUJEMY W KONSOLI (Podgląd danych) ---
                print(f"TIMESTAMP: {ts}")
                print(f"  ACC: x={ax:.2f}, y={ay:.2f}, z={az:.2f}")
                print(f"  GYR: x={gx:.2f}, y={gy:.2f}, z={gz:.2f}")
                print(f"  MAG: x={mx:.2f}, y={my:.2f}, z={mz:.2f}")
                print("-" * 30)

                # --- WSTRZYKNIĘCIE DO C++ ---
                # Nowy format payloadu dla fuzji danych.
                # Kolejność: ax ay az gx gy gz mx my mz timestamp
                # Razem 10 wartości oddzielonych spacją.
                
                payload = (
                    f"{ax:.4f} {ay:.4f} {az:.4f} "
                    f"{gx:.4f} {gy:.4f} {gz:.4f} "
                    f"{mx:.4f} {my:.4f} {mz:.4f} "
                    f"{ts}"
                )
                
                # Wysyłamy pakiet UDP
                udp_socket.sendto(payload.encode('utf-8'), (CPP_HOST, CPP_PORT))

            except Exception as e_inner:
                print(f"!!! BŁĄD PRZETWARZANIA: {e_inner}")
                # print(f"Data: {message}") 

    except websockets.exceptions.ConnectionClosed:
        print(f"Client {websocket.remote_address} disconnected")
    except Exception as e_outer:
        print(f"!!! KRYTYCZNY BŁĄD SERWERA: {e_outer}")

async def main():
    async with websockets.serve(handler, "0.0.0.0", 8081):
        print("WebSocket server started (Raw Sensors Mode).")
        print("Listening on ws://0.0.0.0:8081")
        await asyncio.Future()  # Run forever

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("Server stopped.")