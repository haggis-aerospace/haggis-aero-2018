from dronekit import connect
import dronekit
import exceptions
import socket
import time
from navigation import NavClass
from threading import Thread
from binding import lib as Clib


nav = None
vehicle = None
lib = None

def main():
    global nav
    global vehicle
    global lib

    # Get some vehicle attributes (state)
    print "\nConnected Successfully"
    print " GPS: %s" % vehicle.gps_0
    print " Battery: %s" % vehicle.battery
    print " Last Heartbeat: %s" % vehicle.last_heartbeat
    print " Is Armable?: %s" % vehicle.is_armable
    print " System status: %s" % vehicle.system_status.state
    print " Mode: %s" % vehicle.mode.name    # settable

    #Limit vehicles speed (m/s)
    vehicle.airspeed = 2
    vehicle.groundspeed = 2
    print "Max Speed Set"

    #Loop contains all instructions drone will execute
    while True:

        #Wait for mode to be set to guided
        while vehicle.mode.name != "GUIDED":
            time.sleep(2)
        if vehicle.armed == False: # Arm drone and climb to 10m once in guided mode
            print "Arming..."
            nav.arm_and_takeoff(10)
            nav.goto(1, 1)

        #Main Program execution code
        if vehicle.mode.name == "GUIDED":

            #Test code
            for i in range(1,3):
                flySquare(5)

            nav.goto(20,30)
            nav.goto(-40,0)

            nav.returnToLaunch()

            time.sleep(0.2)
        else:
            time.sleep(3) #If not in guided mode, wait

    # Close vehicle object before exiting script
    vehicle.close()
    print("Completed")


#Spin 180 degrees
def spinRound():
    nav.condition_yaw(180, relative=True)

#Fly in a square
def flySquare(dist=5):
    nav.goto(dist, 0, 0.5, 1)
    nav.goto(0, dist, 0.5, 1)
    nav.goto(dist*-1, 0, 0.5, 1)
    nav.goto(0, dist*-1, 0.5, 1)



def initilize():
    global vehicle
    global nav
    global lib

    try:
        print "Connecting to vehicle..."
        vehicle = connect('127.0.0.1:14550', wait_ready=True)  #Connect to vehicle and initialize home location
        vehicle.home_location = vehicle.location.global_frame

        print "Initializing C++ bindings"
        lib = Clib()
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
