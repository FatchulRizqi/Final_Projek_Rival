#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from std_msgs.msg import String
import asyncio
from bleak import BleakClient
import threading

# Variabel global untuk menyimpan pesan terakhir dari ROS 2
pesan_terakhir = b"KOSONG\n"


class BleakBridgeNode(Node):
    def __init__(self):
        super().__init__('bleak_bridge_node')
        # Subscriber: Mendengarkan topik dari ROS 2 (misal dari node stik PS4)
        self.sub = self.create_subscription(
            String,
            '/perintah_ke_stm32',
            self.ros_callback,
            10
        )
        self.get_logger().info("Node ROS 2 Aktif: Menunggu koneksi Bleak...")

    def ros_callback(self, msg):
        global pesan_terakhir
        # Saat ada topik masuk dari ROS, simpan ke variabel global agar dikirim oleh Bleak
        pesan_terakhir = (msg.data + "\n").encode('utf-8')
        self.get_logger().info(f"ROS menerima perintah: {msg.data}")


async def bleak_loop(node):
    global pesan_terakhir

    MAC_ADDR = "E0:25:F6:69:16:04"  # Ganti dengan MAC Address modul Bluetooth kamu
    CHAR_UUID = "0000ffe1-0000-1000-8000-00805f9b34fb"  # UUID TX/RX BLE

    # Looping abadi selama ROS 2 tidak dimatikan (Ctrl+C)
    while rclpy.ok():
        node.get_logger().info(f"Mencoba menyambungkan Bleak ke {MAC_ADDR}...")
        try:
            async with BleakClient(MAC_ADDR) as client:
                node.get_logger().info("✅ BLEAK TERKONEKSI! Memulai pengiriman data...")

                # Looping pengiriman data selama koneksi tidak putus
                while rclpy.ok() and client.is_connected:
                    if pesan_terakhir != b"KOSONG\n":
                        # Kirim data ke STM32
                        await client.write_gatt_char(CHAR_UUID, pesan_terakhir)
                        # Reset pesan setelah terkirim agar tidak spam
                        pesan_terakhir = b"KOSONG\n"

                    # Beri jeda kecil agar CPU tidak 100% dan memberi waktu Bleak bernapas
                    await asyncio.sleep(0.1)

        except Exception as e:
            node.get_logger().error(f"❌ Koneksi Bleak terputus/gagal: {e}")
            node.get_logger().info("Mencoba ulang dalam 3 detik...")
            await asyncio.sleep(3.0)  # Jeda sebelum reconnect otomatis


def main(args=None):
    rclpy.init(args=args)
    node = BleakBridgeNode()

    # 1. Jalankan ROS 2 Spin di Thread terpisah agar tidak memblokir Bleak
    spin_thread = threading.Thread(
        target=rclpy.spin, args=(node,), daemon=True)
    spin_thread.start()

    # 2. Jalankan Looping Bleak di Thread utama
    try:
        asyncio.run(bleak_loop(node))
    except KeyboardInterrupt:
        node.get_logger().info("Mematikan Node...")
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
