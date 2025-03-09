from smbus2 import SMBus
import time
from datetime import datetime
import csv

print("Beggining I2C Testbench - Capstone Prototype")

I2C_BUS = 7
bus = SMBus(I2C_BUS)
slaveAddress = 0x22
print("SLAVE ADDR ASSIGNED")
correct = True
i = 0

def readNumber(i):
	try:
		bus.write_byte(slaveAddress, 0x00)
		time.sleep(0.01)
		print("message working")
		response = bus.read_byte(slaveAddress)
		time.sleep(0.01)
		print(i, response)
		ts = time.time()
		
		now = datetime.now()
		readable_time = now.strftime("%Y-%m-%d %H:%M:%S")
		print(readable_time)

		if(response == 0):
			state = "DRY"
		elif(response == 1):
			state = "HAZARD"
		elif(response == 16):
			state = "CRITICAL"
		else:
			state = "UNKNOWN"

		print(state)


		with open('testing1.csv', 'a', newline = '') as csvfile:
			writer = csv.writer(csvfile)
			writer.writerow([i, readable_time, response, state])
		return 1
	
	except OSError as e:
		print("I2C Comm Error")
		time.sleep(2)
		readNumber()


if __name__ == "__main__":
	while True:
		i += 1
		readNumber(i)
		time.sleep(1)