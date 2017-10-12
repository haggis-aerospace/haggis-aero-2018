from dronekit import connect
import dronekit
import exceptions
import socket
import time
import navigation as nav
from threading import Thread



def main(vehicle):
    print "Starting simulator (SITL)"

    # Get some vehicle attributes (state)
    print "Connected Successfully"
    print " GPS: %s" % vehicle.gps_0
    print " Battery: %s" % vehicle.battery
    print " Last Heartbeat: %s" % vehicle.last_heartbeat
    print " Is Armable?: %s" % vehicle.is_armable
    print " System status: %s" % vehicle.system_status.state
    print " Mode: %s" % vehicle.mode.name    # settable

    #Enable communication through another Ground station


    #Limit vehicles speed (m/s)
    vehicle.airspeed = 2
    vehicle.groundspeed = 2

    #Loop contains all instructions drone will execute
    while True:

        #Wait for mode to be set to guided
        while vehicle.mode.name != "GUIDED":
            time.sleep(2)
        if vehicle.armed == False: # Arm drone and climb to 10m once in guided mode
            nav.arm_and_takeoff(vehicle, 10)
            nav.goto(vehicle, 1, 1)

        #Main Program execution code
        if vehicle.mode.name == "GUIDED":

            #Test code
            for i in range(1,3):
                flySquare(5)

            nav.goto(vehicle, 20,30)
            nav.goto(vehicle, -40,0)

            nav.returnToLaunch(vehicle)

            time.sleep(0.2)
        else:
            time.sleep(3) #If not in guided mode, wait

    # Close vehicle object before exiting script
    vehicle.close()
    print("Completed")


#Spin 180 degrees
def spinRound():
    nav.condition_yaw(vehicle, 180, relative=True)

#Fly in a square
def flySquare(dist=5):
    nav.goto(vehicle, dist, 0, 0.5, 1)
    nav.goto(vehicle, 0, dist, 0.5, 1)
    nav.goto(vehicle, dist*-1, 0, 0.5, 1)
    nav.goto(vehicle, 0, dist*-1, 0.5, 1)



def initilize():
    try:
        vehicle = connect('127.0.0.1:14550', wait_ready=True)  #Connect to vehicle and initialize home location
        vehicle.home_location = vehicle.location.global_frame
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
    main(vehicle)

