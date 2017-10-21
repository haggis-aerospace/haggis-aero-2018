from dronekit import VehicleMode, LocationGlobal, LocationGlobalRelative
import dronekit
from pymavlink import mavutil
import time
import math
import threading

class NavClass:

    vehicle=None

    def __init__(self, vehicleRef):
        global vehicle
        vehicle = vehicleRef

    def returnToLaunch(self, height=30,land=True):
        global vehicle

        """
        Returns to the original arm location. Vehicle will first climb to height set by height
        parameter
        :param vehicle:
        :param height:
        :param land:
        :return:
        """

        print "Returning to home location"
        currentLoc = vehicle.location.global_relative_frame
        currentLoc.alt = height     #Modifying altitude to climb at current location
        vehicle.simple_goto(currentLoc)

        while True:
            print " Altitude: ", vehicle.location.global_relative_frame.alt
            #Break a*nd return from function just below target altitude.
            if vehicle.location.global_relative_frame.alt >= height*0.95:
                print "Reached target altitude"
                break
            time.sleep(1)

        #Gets home location and altitude
        homeLoc = LocationGlobalRelative(vehicle.home_location.lat, vehicle.home_location.lon, height)
        self.gotoPoint(homeLoc)

        if land:
            print "Landing"
            vehicle.send_mavlink(vehicle.message_factory.command_long_encode(0, 0, mavutil.mavlink.MAV_CMD_NAV_LAND, 0, 0, 0, 0, 0, 0, 0, 0))


    def arm_and_takeoff(self, aTargetAltitude):
        global vehicle

        """
        Arms vehicle and fly to aTargetAltitude.
        """

        print "Basic pre-arm checks"
        # Don't try to arm until autopilot is ready
        while not vehicle.is_armable:
            print " Waiting for vehicle to initialise..."
            time.sleep(1)

        print "Arming motors"
        # Copter should arm in GUIDED mode
        vehicle.mode = VehicleMode("GUIDED")
        vehicle.home_location = vehicle.location.global_frame
        vehicle.armed = True

        # Confirm vehicle armed before attempting to take off
        while not vehicle.armed:
            print " Waiting for arming..."
            time.sleep(1)

        print "Taking off!"
        vehicle.simple_takeoff(aTargetAltitude) # Take off to target altitude

        # Wait until the vehicle reaches a safe height before processing the goto (otherwise the command
        #  after Vehicle.simple_takeoff will execute immediately).
        while True:
            print " Altitude: ", vehicle.location.global_relative_frame.alt
            #Break and return from function just below target altitude.
            if vehicle.location.global_relative_frame.alt >= aTargetAltitude*0.95:
                print "Reached target altitude"
                break
            time.sleep(1)


    def gotoPoint(self, targetLocation, accuracy=0.5, close=1.5):
        global vehicle

        """
            Moves the vehicle to a position dNorth metres North and dEast metres East of the current position.
            By default it uses the standard method: dronekit.lib.Vehicle.simple_goto().

            The method reports the distance to target every two seconds.
        """

        vehicle.simple_goto(targetLocation)

        timeClose = 0
        while vehicle.mode.name == "GUIDED":  # Stop action if we are no longer in guided mode.
            remainingDistance = self.get_distance_metres(vehicle.location.global_frame, targetLocation)
            print "Distance to target: ", remainingDistance
            if remainingDistance <= accuracy:
                print "Reached target"
                break;
            elif remainingDistance < close:  # Resend command if drone has been close to point for >4 seconds
                if timeClose >= 4:
                    print "Resending location data"
                    vehicle.simple_goto(targetLocation)
                    timeClose = 0
                else:
                    timeClose += 2
            time.sleep(2)


    def get_location_metres(self, original_location, dNorth, dEast):
        global vehicle

        """
        Returns a LocationGlobal object containing the latitude/longitude `dNorth` and `dEast` metres from the
        specified `original_location`. The returned LocationGlobal has the same `alt` value
        as `original_location`.

        The function is useful when you want to move the vehicle around specifying locations relative to
        the current vehicle position.

        The algorithm is relatively accurate over small distances (10m within 1km) except close to the poles.

        For more information see:
        http://gis.stackexchange.com/questions/2951/algorithm-for-offsetting-a-latitude-longitude-by-some-amount-of-meters
        """
        earth_radius = 6378137.0  # Radius of "spherical" earth
        # Coordinate offsets in radians
        dLat = dNorth / earth_radius
        dLon = dEast / (earth_radius * math.cos(math.pi * original_location.lat / 180))

        # New position in decimal degrees
        newlat = original_location.lat + (dLat * 180 / math.pi)
        newlon = original_location.lon + (dLon * 180 / math.pi)
        if type(original_location) is LocationGlobal:
            targetlocation = LocationGlobal(newlat, newlon, original_location.alt)
        elif type(original_location) is LocationGlobalRelative:
            targetlocation = LocationGlobalRelative(newlat, newlon, original_location.alt)
        else:
            raise Exception("Invalid Location object passed")

        return targetlocation


    def goto(self, dNorth, dEast, accuracy=0.5, close=2):
        global vehicle

        """
            Moves the vehicle to a position dNorth metres North and dEast metres East of the current position.
            By default it uses the standard method: dronekit.lib.Vehicle.simple_goto().

        """

        print "Going to " + str(dNorth) + " " + str(dEast)
        currentLocation = vehicle.location.global_relative_frame
        targetLocation = self.get_location_metres(currentLocation, dNorth, dEast)
        self.gotoPoint(targetLocation, accuracy, close)


    def get_distance_metres(self, aLocation1, aLocation2):
        global vehicle

        """
        Returns the ground distance in metres between two LocationGlobal objects.

        This method is an approximation, and will not be accurate over large distances and close to the
        earth's poles. It comes from the ArduPilot test code:
        https://github.com/diydrones/ardupilot/blob/master/Tools/autotest/common.py
        """
        dlat = aLocation2.lat - aLocation1.lat
        dlong = aLocation2.lon - aLocation1.lon
        return math.sqrt((dlat * dlat) + (dlong * dlong)) * 1.113195e5


    def condition_yaw(self, heading, relative=False):
        global vehicle

        """
        Adjusts the vehicles heading to desired angle; either absolute angle or relative angle
        determined by the relative param
        :param vehicle:
        :param heading:
        :param relative:
        :return:
        """
        if relative:
            is_relative=1 #yaw relative to direction of travel
        else:
            is_relative=0 #yaw is an absolute angle
        print "Turning to heading " + str(heading)
        # create the CONDITION_YAW command using command_long_encode()
        msg = vehicle.message_factory.command_long_encode(0, 0, mavutil.mavlink.MAV_CMD_CONDITION_YAW, 0, heading, 0, 1, is_relative, 0, 0, 0)
        # send command to vehicle
        vehicle.send_mavlink(msg)
