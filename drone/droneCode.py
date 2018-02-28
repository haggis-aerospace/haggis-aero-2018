from dronekit import connect
import dronekit
import exceptions
import socket
import time
from navigation import NavClass
from threading import Thread
from tcpServer import Server

# from binding import lib as Clib
# from binding import Letter


nav = None
vehicle = None
server = None

def main(rotation=False, goTo=False, run=False, alt=False):
    global nav, vehicle, server

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

    #Loop contains all instructions drone will execute
    while True:

        #Wait for mode to be set to guided
        while vehicle.mode.name != "GUIDED" or vehicle.armed == False:
            time.sleep(2)
            print "Waiting for Arm in Guided..."
        #if vehicle.armed == False: # Arm drone and climb to 10m once in guided mode
        #    print "Arming..."
        #    nav.arm_and_takeoff(2)

        #Main Program execution code
        if vehicle.mode.name == "GUIDED":
            #Test code
            letter = server.getLastLetter()
            if letter.width > 0 and letter.height > 0:
                if(rotation):
                    lookAtLetter()
                if(goTo):
                    gotoLetter()
                elif(run):
                    runFromLetter()
                if(alt):
                    letterHeight()

            time.sleep(0.2)
        else:
            time.sleep(3) #If not in guided mode, wait

    # Close vehicle object before exiting script
    vehicle.close()
    print("Completed")


def lookAtLetter():
    global server, nav
    pos = server.getLastLetter().x
    while (pos < 40 or pos > 60) and server.getLastLetter().width > 0:
        if pos > 60:
            print "Turning Left"
            nav.condition_yaw(355, True)
        elif pos < 40:
            print "Turning Right"
            nav.condition_yaw(005, True)
        time.sleep(0.5)
        pos = server.getLastLetter().x


def gotoLetter():
    global server,nav
    time.sleep(2)
    baseSize = server.getLastLetter().width * server.getLastLetter().height
    while server.getLastLetter().width > 0:
        lookAtLetter()
        if server.getLastLetter().width * server.getLastLetter().height < baseSize*0.9:
            print "Moving Forward"
            nav.send_body_ned_velocity(0.15,0,0,300)
        elif server.getLastLetter().width * server.getLastLetter().height > baseSize*1.1:
            print "Moving Backwards"
            nav.send_body_ned_velocity(-0.15,0,0,300)
        time.sleep(0.5)


def runFromLetter():
    global server,nav
    time.sleep(2)
    baseSize = server.getLastLetter().width * server.getLastLetter().height
    while server.getLastLetter().width > 0:
        lookAtLetter()
        if server.getLastLetter().width * server.getLastLetter().height > baseSize*1.1:
            print "Moving Backwards"
            nav.send_body_ned_velocity(-0.15,0,0,300)
        time.sleep(0.5)

def letterHeight():
    global server,nav
    lookAtLetter()
    pos = server.getLastLetter().y
    while (pos < 25 or pos > 75) and server.getLastLetter().width > 0:
        if pos > 60:
            print "Letter Decending"
            nav.altChangeRelative(-0.2, True)
        elif pos < 40:
            print "Letter Climbing"
            nav.altChangeRelative(0.2, True)
        lookAtLetter()
        pos = server.getLastLetter().y
        time.sleep(0.5) 

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


def initilize(rotation=True,goTo=False, run=False,alt=False):
    global vehicle
    global nav
    global server

    try:
        print "Connecting to vehicle..."
        vehicle = connect('/dev/ttyACM0', wait_ready=True,baud=57600)  # Connect to vehicle and initialize home location
        # vehicle = connect('127.0.0.1:14550', wait_ready=True)  #Connect to vehicle and initialize home location
        vehicle.home_location = vehicle.location.global_frame

        print "Waiting for client to connect..."
        server = Server("127.0.0.1", 5463)
        while not server.isClientActive():
            time.sleep(1)

        print "\nInitializing navigation"
        nav = NavClass(vehicle)
        main(rotation, goTo, run, alt)

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
