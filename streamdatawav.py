import serial
import time

def send_wav_file_to_arduino(file_path, ser):
    # Open the WAV file and skip the header
    with open(file_path, 'rb') as f:
        f.seek(44)
        data = f.read()
        print("SENDING to arduino", data)
        ser.write(data)
        print("sent successfully")

def read_output_from_arduino(ser):
    print("IN  arduino")
    while True:
        if ser.in_waiting > 0:
            line = ser.readline().decode('latin-1').strip()
            print("RECEIVING from Arduino:", line)
        time.sleep(0.1)

def main():
    # port = '/dev/cu.SLAB_USBtoUART'  # Serial port connected to ESP32
    port = '/dev/cu.usbmodem14101'  # Serial port connected to Arduino & Raspberry Pi
    baud_rate = 115200 
    # file_path = '1cb788bc_nohash_0.wav'  # Path to the WAV file
    file_path = 'bfd26d6b_nohash_0.wav' 
    # file_path = 'test_output.pdm' #down converted pdm
    ser = serial.Serial(port, baud_rate)
    time.sleep(2) 

    try:
        send_wav_file_to_arduino(file_path, ser)

        read_output_from_arduino(ser)
    except KeyboardInterrupt:
        print("KeyboardInterrupt detected. Closing serial connection.")
        ser.close()

if __name__ == "__main__":
    main()
