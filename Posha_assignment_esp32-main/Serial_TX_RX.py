#Basic code to send data on UART and then receive it and then print it


import serial
import argparse
import time

data = ("Finance Minister Arun Jaitley Tuesday hit out at former RBI governor Raghuram Rajan for predicting that the next banking crisis would be triggered by MSME lending, saying postmortem is easier than taking action when it was required. Rajan, who had as the chief economist at IMF warned of impending financial crisis of 2008, in a note to a parliamentary committee warned against ambitious credit targets and loan waivers, saying that they could be the sources of next banking crisis. Government should focus on sources of the next crisis, not just the last one. In particular, government should refrain from setting ambitious credit targets or waiving loans. Credit targets are sometimes achieved by abandoning appropriate due diligence, creating the environment for future NPAs, Rajan said in the note Both MUDRA loans as well as the Kisan Credit Card, while popular, have to be examined more closely for potential credit risk. Rajan, who was RBI governor for three years till September 2016, is currently.\n")

def print_serial(name):

    try:
    #     serial_port = serial.Serial(name,2400)
    #     print(f"The Port name is {serial_port.name}")
        ser = serial.Serial(name, 2400)
        ser.write(str.encode(data))
        print("Data Sent")
        print("Waiting for data\n")
        stime = time.time()
        lines = ser.readline()              
        #Readline waits for \n character, so if the data does not contain '\n' towards the end, this will block the code. 
        #We must ensure that there is a '\n' at the end of the data or use a timeout in this code

        ctime = time.time()
        transmission_speed = (len(lines))/(ctime - stime)
        print(lines.decode('utf-8'))
        print("reception speed =  bytes/s",transmission_speed)

    except:
        print("ERROR")
        print("check port")
        exit() 

ap = argparse.ArgumentParser()                                                    
#So we can provide a port when we run the script
ap.add_argument("-p","--port",required = True, help = "Enter Port Name")
args = vars(ap.parse_args())
PORT = args['port']
while(1):
    print_serial(PORT)
    time.sleep(1)
