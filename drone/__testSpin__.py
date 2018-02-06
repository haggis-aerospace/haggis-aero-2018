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
        while vehicle.mode.name != "GUIDED":
            time.sleep(2)
            print "Waiting for Arm..."

        #Main Program execution code
        if vehicle.mode.name == "GUIDED":
            spinRound()
            time.sleep(8)
        else:
            time.sleep(3) #If not in guided mode, wait

    # Close vehicle object before exiting script
    vehicle.close()
    print("Completed")


#Spin 180 degrees
def spinRound():
    nav.condition_yaw(180, relative=True)


def initilize():
    global vehicle
    global nav
    global server

    try:
        print "Connecting to vehicle..."
        vehicle = connect('/dev/ttyACM0', wait_ready=True,baud=57600)  # Connect to vehicle and initialize home location
      #  vehicle = connect('127.0.0.1:14550', wait_ready=True)  #Connect to vehicle and initialize home location
        vehicle.home_location = vehicle.location.global_frame

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

initilize()
