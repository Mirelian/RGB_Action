import serial
import time

# Replace 'COM3' with the correct port for your system
port = 'COM4'
baudrate = 9600
timeout = 1

# Initialize the serial connection
ser = serial.Serial(port, baudrate, timeout=timeout)

def send_command(command):
    """Send a command to the power supply and read the response."""
    ser.write((command + '\n').encode())
    time.sleep(0.1)
    response = ser.read(ser.inWaiting()).decode().strip()
    return response

def set_voltage(voltage):
    """Set the voltage of the power supply."""
    command = f'VSET1:{voltage:.2f}'
    send_command(command)

try:
    while True:
        user_input = input("Enter voltage (e.g., 12.00) or 'q' to quit: ")
        if user_input.lower() == 'q':
            break
        try:
            voltage = float(user_input)
            set_voltage(voltage)
            print(f"Set voltage to {voltage:.2f} V")
            print("Current Voltage:", send_command('VOUT1?'))
        except ValueError:
            print("Invalid input. Please enter a number.")
finally:
    ser.close()