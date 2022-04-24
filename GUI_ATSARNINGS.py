import webbrowser
import requests
import time
import threading
import sys
import signal
import serial
import lcddriver
import socket
import struct
import fcntl
from xml.etree import ElementTree

class TEWS:
    
    BMKG_stat = "Disconnected"
    SMS_stat = "Not Ready"

    def __init__(self, Tanggal, Jam, Latitude, Longtitude, Magnitude, Kedalaman, Wilayah1, Wilayah2, Wilayah3, Wilayah4, Wilayah5, Potensi):
        #self.Bmkg_stat = BMKG_s
        #self.sms_stat = SMS_s
        self.Tanggal = Tanggal
        self.Jam = Jam
        self.Latitude = Latitude
        self.Longtitude = Longtitude
        self.Magnitude = Magnitude
        self.Kedalaman = Kedalaman
        self.Wilayah1 = Wilayah1
        self.Wilayah2 = Wilayah2
        self.Wilayah3 = Wilayah3
        self.Wilayah4 = Wilayah4
        self.Wilayah5 = Wilayah5
        self.Potensi = Potensi

    def DisplayLCD(self):
        global z
        z += 1
        if len(self.BMKG_stat) > 9 or len(self.SMS_stat) > 5:
            lcd.lcd_print("BMKG Server Status",0)
            if len(self.BMKG_stat) <= 9:
                lcd.lcd_print("> Connected   ",1)
            else:
                lcd.lcd_print("> Disconnected",1)
            lcd.lcd_print("GSM Module Status",2)
            if len(self.SMS_stat) <= 5:
                lcd.lcd_print("> Ready    ",3)
            else:
                lcd.lcd_print("> Not Ready",3)
        elif self.Potensi.find('tidak') == -1:
            LenSend = self.Wilayah1[:(self.Wilayah1.find('km') + 3)]
            lcd.lcd_clear()
            lcd.lcd_print("BERPOTENSI TSUNAMI!",0)
            lcd.lcd_print(self.Latitude + " " + self.Magnitude,1)
            lcd.lcd_print(self.Longtitude + " " + LenSend,2)
        else:
            lcd.lcd_print("%s" %time.strftime("%H:%M:%S  %m/%d/%Y"),0)
            lcd.lcd_print("Update Gempa      ",1)
            if z == 1:
                lcd.lcd_print("                    ",2)
                lcd.lcd_print("                    ",3)
                lcd.lcd_print(self.Tanggal,2)
                lcd.lcd_print(self.Jam,3)
            elif z == 2:
                lcd.lcd_print("                    ",2)
                lcd.lcd_print("                    ",3)
                lcd.lcd_print(self.Latitude,2)
                lcd.lcd_print(self.Longtitude,3)
            elif z == 3:
                lcd.lcd_print("                    ",2)
                lcd.lcd_print("                    ",3)
                lcd.lcd_print("MAG   : " + self.Magnitude ,2)
                lcd.lcd_print("DEPTH : " + self.Kedalaman,3)
            elif z == 4:
                if len(self.Potensi) > 18:
                    lcd.lcd_print("                    ",2)
                    lcd.lcd_print("                    ",3)
                    lcd.lcd_print("TIDAK BERPOTENSI",2)
                    lcd.lcd_print("TSUNAMI",3)
        if z == 4:
            z = 0
        return

    def Show_html(self):

        f = open('/var/www/html/index.html','w')

        message = """<html>
        <head>
        <script>
        var myVar = setInterval(myTimer, 1000);

        function myTimer() {
        var d = new Date();
        var t = d.toLocaleTimeString();
        document.getElementById("times").innerHTML = t;
        }
        </script>
        <h1>Tsunami Early Warning System Tersingkronikasi BMKG dan Tempat Ibadah</h1>
        <h2>PKM-KC UNY</h2>
        </head>
        <body>
        <hr></hr>
        <h2>BMKG Server """ + self.BMKG_stat + """</h2>
        <h2>GSM Module """ + self.SMS_stat + """</h2>
        <hr></hr>
        <h2 id="times"></h2>
        <table>
        <hr>
        <td><strong>Data BMKG</strong></td>
        <td><strong>Status Alarm</strong></td>
        </hr>
        <tr>
        <td>Tanggal : """ + self.Tanggal + """<br/>
        Waktu : """ + self.Jam + """<br/>
        Tanggal : """ + self.Tanggal + """<br/>
        Lintang : """ + self.Latitude + """<br/>
        Bujur : """ + self.Longtitude + """<br/>
        Tanggal : """ + self.Tanggal + """
        </td>

        <td>Siap Pakai</td>
        </tr>
        </table>
        <hr></hr>
        <h2>TIDAK BERPOTENSI TSUNAMI</h2>
        </body>
        </html>"""

        f.write(message)


def checking():
    global sms_stat
    result = False
    print ('Checking SIM900A Connection')
    com.write('AT\r\n')
    time.sleep(0.1)
    count = 0
    while count <= 2:
        if com.inWaiting() > 0:
            confirmCheck = com.readline()
            if confirmCheck.find('OK') != -1:
                print ('SIM900A Modul Connected')
                TEWS.SMS_stat = "Ready"
                count = 3
                result = True         
        count += 1

    if result == False:
        test = "OK"
        print ('SIM900A Error Connection')
        TEWS.SMS_stat = "Not Ready"
    return
    
def sendMessage(Id,Msg):
    success = False
    print ('Sending Message')
    com.write(str.encode('AT+CMGF=1\r\n'))
    time.sleep(0.1)
    #com.write("AT+CMGS=\"+6281392383288\""+"\r\n")
    #com.write("AT+CMGS=\"+6282313525173\""+"\r\n")
    com.write(str.encode('AT+CMGS=\"' + Id + '\"\r\n'))
    time.sleep(0.1)
    com.write(Msg+chr(26))
    time.sleep(0.1)
    count = 0
    while count < 8:
        if com.inWaiting() > 0:
            confirmSend = com.readline()
            if confirmSend.find('+CMGS=') != -1:
                print ('SMS has been sended')
                lcd.lcd_print("SMS Terkirim      ",3)
                success = True
                return
    count += 1
    if success == False:
        print ('SMS cannot send')
        print (confirmSend)
        lcd.lcd_print("SMS Gagal Terkirim",3)
        #com.close()
        #sys.exit()

def listeningMessage():
    print ('Listening Message')
    while True:
        if com.inWaiting() > 0:
            dataSerial = com.readline()
            if dataSerial.find('+CMT:') != -1:
                print ('Message Receive')
                rec = 1
            findMessage = dataSerial.find('$')
            if findMessage != -1 and rec == 1:
                split = dataSerial[findMessage+1:]
                print (' ')
                print ('Incoming Message ')
                print (split)
                rec = 0

        global stop_threads
        if stop_threads:
            sys.exit()
            break
    
def RequestBMKG():
    while True:
        global lintang
        global bujur
        try:
            #response = requests.get('http://data.bmkg.go.id/autogempa.xml')
            response = requests.get('http://127.0.0.1/autogempa.xml')
    
            tree = ElementTree.fromstring(response.content)

            tgl = tree[0][0].text
            jam = tree[0][1].text
            #coordinate= tree[0][2][0].text
            lintang = tree[0][3].text
            bujur = tree[0][4].text
            magnitude = tree[0][5].text
            potensi = tree[0][13].text
            kedalaman = tree[0][6].text
            wilayah1 = tree[0][8].text
            wilayah2 = tree[0][9].text
            wilayah3 = tree[0][10].text
            wilayah4 = tree[0][11].text
            wilayah5 = tree[0][12].text

            TEWS.BMKG_stat = "Connected"
            print('BMKG Server Connected')
            
            raw_data = TEWS(tgl,jam,lintang,bujur,magnitude,kedalaman,wilayah1,wilayah2,wilayah3,wilayah4,wilayah5,potensi)
            raw_data.Show_html()
            raw_data.DisplayLCD()
            
            #if lintang.find('LS') != -1:
            #    lintangUtara = True
            #elif lintang.find('LU') != -1:
            #    lintangUtara = False

            latitude = lintang.split(' ',1)

            MagSend = magnitude[:magnitude.find('SR')]
            DepSend = kedalaman[:kedalaman.find('Km')]
            LenSend = wilayah1[:wilayah1.find('km')]

            Msg = '(' + MagSend + "!" + DepSend + '&' + LenSend + ')'
            Pesan = "WASPADA POTENSI BAHAYA TSUNAMI!\n\rMagnitude :" + MagSend + " SR, Kedalaman : " + DepSend + "Km, Jarak : " + LenSend + " Km"
	
            if potensi.find('tidak') != -1:
                checking()
                SEND = True
            else:
                if lintang.find('LS') != -1:
                    print('Bahaya Tsunami')
                    coorY = lintang[:lintang.find('.')]
                    coorX = bujur[:bujur.find('.')]
                    if int(coorY) >= 7 and int(coorX) >= 101 and int(coorX) <= 116 and SEND == True:
                        sendMessage('+6282313525173',Msg)
                        #sendMessage('+6285742347629',Msg)
			time.sleep(5)
                        sendMessage('+6281392383288',Pesan)
			time.sleep(5)
			sendMessage('+6281578553523',Pesan)
			time.sleep(5)
			sendMessage('+6285335689259',Pesan)
			time.sleep(5)
			sendMessage('+628112509575',Pesan)
                        SEND = False

            #print ('Server BMKG Connected')
            #print ('Koordinat Gempa ' + longtitude + ' , ' + str(abs(float(latitude))))
            #print ('Kekuatan Gempa ' + magnitude)
            #print ('Potensi Gempa ' + potensi)
            #print (' ')

            time.sleep(5)

            global stop_threads
            if stop_threads:
                sys.exit()
                break
            
        except Exception as e:
            print (e)
            print('BMKG Disconnect') 
            TEWS.BMKG_stat = "Disconnected"
            time.sleep(2)

def main_quit(obj):
    print ('Stop Program')
    global stop_threads
    stop_threads = True
    
def keyboardInterruptHandler(signal, frame):
    print ('Keyboard Interrupt...')
    global stop_threads
    stop_threads = True
    sys.exit()
    com.close()

# Main Thread

lcd = lcddriver.lcd()

com = serial.Serial('/dev/ttyS0', baudrate=19200,
	            parity=serial.PARITY_NONE,
                    stopbits=serial.STOPBITS_ONE,
                    bytesize=serial.EIGHTBITS)

lcd.lcd_print("   Tsunami Early", 0)
lcd.lcd_print("   Warning System", 1)
lcd.lcd_print("  Tersinkronisasi", 2)
lcd.lcd_print("  BMKG dan Masjid", 3)

time.sleep(2)

lcd.lcd_clear()

wilayah1 = 0

global z
global stop_threads
stop_threads = False
z=0

t_request = threading.Thread(target=RequestBMKG)
#t_listenSMS = threading.Thread(target=listeningMessage)

t_request.start()
#t_listenSMS.start()

signal.signal(signal.SIGINT, keyboardInterruptHandler)
