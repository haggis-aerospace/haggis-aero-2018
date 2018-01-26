from dronekit import connect
import dronekit
import exceptions
import socket
import time
from navigation import NavClass
from threading import Thread
from binding import lib as Clib
from binding import Letter

nav = None
vehicle = None
lib = None

def main():
    global nav, vehicle, lib

    # Get some vehicle attributes (state)
    print "\nConnected Successfully"
    print " GPS: %s" % vehicle.gps_0
    print " Battery: %s" % vehicle.battery
    print " Last Heartbeat: %s" % vehicle.last_heartbeat
    print " Is Armable?: %s" % vehicle.is_armable
    print " System status: %s" % vehicle.system_status.state
    print " Mode: %s" % vehicle.mode.name    # settable

    #Limit vehicles speed (m/s)
    vehicle.airspeed = 1
    vehicle.groundspeed = 1
    print "Max Speed Set"

    print "Initilizing Camera loop..."
    workerThread = Thread(target=letterLoop)
    workerThread.setDaemon(True)
    workerThread.start()
    time.sleep(3)

    letters = {
        "X": lookAtLetter,
        "Y": gotoLetter,
        #"A": runFromLetter,
        "T": landLetter,
    }

    #Loop contains all instructions drone will execute
    while True:

        #Wait for mode to be set to guided
        while vehicle.mode.name != "GUIDED":
            time.sleep(2)
            print "Waiting for Arm..."
        if vehicle.armed == False: # Arm drone and climb to 10m once in guided mode
            print "Arming..."
            nav.arm_and_takeoff(1.5)

        #Main Program execution code
        if vehicle.mode.name == "GUIDED":

            #Test code
            try:
                letter = lib.getLastLetter().letter
                func = letters[letter]
                func()
            except KeyError:
                None
            time.sleep(0.2)
        else:
            time.sleep(3) #If not in guided mode, wait

    # Close vehicle object before exiting script
    vehicle.close()
    print("Completed")


def lookAtLetter():
    global lib, nav
    pos = lib.getLastLetter().pos
    while (pos < 40 or pos > 60) and lib.getLastLetter().width > 0:
        if pos > 60:
            print "Turning Left"
            nav.condition_yaw(357, True)
        elif pos < 40:
            print "Turning Right"
            nav.condition_yaw(003, True)
        time.sleep(0.1)
        pos = lib.getLastLetter().pos

def gotoLetter():
    global lib,nav
    time.sleep(2)
    baseSize = lib.getLastLetter().avSize
    while lib.getLastLetter().letter == "Y":
        lookAtLetter()
        if lib.getLastLetter().avSize < baseSize*0.9:
            print "Moving Forward"
            nav.send_body_ned_velocity(0.15,0,0,300)
        elif lib.getLastLetter().avSize > baseSize*1.1:
            print "Moving Backwards"
            nav.send_body_ned_velocity(-0.15,0,0,300)

def runFromLetter():
    return None

def landLetter():
    nav.returnToLaunch(10)

#Spin 180 degrees
def spinRound():
    nav.condition_yaw(180, relative=True)

#Fly in a square
def flySquare(dist=5):
    nav.goto(dist, 0, 0.5, 1)
    nav.goto(0, dist, 0.5, 1)
    nav.goto(dist*-1, 0, 0.5, 1)
    nav.goto(0, dist*-1, 0.5, 1)



def letterLoop():
    while True:
        lib.findLetters()
        letter = lib.mostOccouring()
        if letter.width > 0 and letter.height > 0:
            print letter.letter


def initilize():
    global vehicle
    global nav
    global lib

    try:
        print "Connecting to vehicle..."
        vehicle = connect('/dev/ttyACM0', wait_ready=True, baud=57600)  #Connect to vehicle and initialize home location
        vehicle.home_location = vehicle.location.global_frame

        print "Initializing C++ bindings"
        lib = Clib(False)
        lib.libtest()

        print "\nInitializing navigation"
        nav = NavClass(vehicle)

        main()
    # Bad TCP connection
    except socket.error:
        print 'No server exists!'
    # Bad TTY connection
    except exceptions.OSError as e:
        print 'No serial exists!'
    # API Error
    except dronekit.APIException:
        print 'Timeout!'
    # Other error
    except Exception, e:
        print 'Unknown Error Has Occurred During Connection'
        print e.message
