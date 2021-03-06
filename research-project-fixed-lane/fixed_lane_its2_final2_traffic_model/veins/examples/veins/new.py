#Lambda = 1.0 / desired mean
import xml.etree.cElementTree as ET
import random
from random import randint

#prepare inputs
lambd1 = input("Mean of lane 1 : ")
lambd0 = input("Mean of lane 0: ")
num = input("Number of vehicles in one lane : ")
currentTime1 = random.expovariate(1.0/lambd1)
currentTime0 = random.expovariate(1.0/lambd0)

vipDepartureTime = random.uniform(max(lambd1,lambd0)*num/4,max(lambd1,lambd0)*num/2)

#add DOM elements
root = ET.Element("routes")
ET.SubElement(root, "vType", tau="0", accel="2.9", decel="7.5", id="Car", length="4.3", maxSpeed="19.44", sigma="0.0", minGap="-1").text = ""        #for normal vehicles

ET.SubElement(root, "vType", tau="0", accel="2.9", decel="7.5", id="CarVIP", length="6.5", maxSpeed="27.78", sigma="0.0", minGap="-1").text = ""    #for VIP

ET.SubElement(root, "route", id="route0", edges="1to2").text = ""



#if VIP is the first vehicle
if(vipDepartureTime <= currentTime0):
    ET.SubElement(root, "vehicle", color="red", depart=str(vipDepartureTime), departLane="0", arrivalLane="0", id="vip", route="route0", type="CarVIP").text = ""

#else if the VIP is in the middle of the vehicle flow

loop_runner0 = False
loop_runner1 = False
loop_runner = True
lane0_count = 0
lane1_count = 0
while(loop_runner):
    dgap1 = random.expovariate(1.0/lambd1)
    dgap0 = random.expovariate(1.0/lambd0)
    
    if(currentTime0 < currentTime1 and (not loop_runner0)):
        ET.SubElement(root, "vehicle", color="yellow", depart=str(currentTime0), departLane="0", arrivalLane="0", id=str(lane0_count+lane1_count), route="route0", type="Car").text = ""
        if(vipDepartureTime >= currentTime0 and vipDepartureTime < currentTime0 + dgap0):
            ET.SubElement(root, "vehicle", color="red", depart=str(vipDepartureTime), departLane="0", arrivalLane="0", id="vip", route="route0", type="CarVIP").text = ""
        currentTime0 = currentTime0 + dgap0
        lane0_count += 1
    elif(not loop_runner1):
        ET.SubElement(root, "vehicle", color="green", depart=str(currentTime1), departLane="1", arrivalLane="1", id=str(lane1_count+lane0_count), route="route0", type="Car").text = ""
        currentTime1 = currentTime1 + dgap1
        lane1_count += 1
    if(lane0_count == num): loop_runner0 = True
    if(lane1_count == num): loop_runner1 = True
    if(loop_runner0 and loop_runner1): loop_runner = False
    
tree = ET.ElementTree(root)
tree.write("hello.rou.xml")


#<routes>
#<vType tau="0" accel="1.0" decel="2.0" id="Car" length="2.0" maxSpeed="10.0" sigma="0.0" minGap="-1"/>
#<vType tau="0" accel="10.0" decel="10.0" id="Car1" length="2.0" maxSpeed="180.0" sigma="0.0" minGap="-1"/>
#<route id="route0" edges="1to2"/>
#<flow id="1" begin="0" end="50" depart="0" type="Car" period="2.5" departLane="1" arrivalLane="1" route="route0"/>
#<flow id="2" begin="4" end="54" depart="0" type="Car" period="2.5" departLane="0" arrivalLane="0" route="route0"/>
#<vehicle color="red" depart="55" departLane="1" arrivalLane="1" id="vip" route="route0" type="Car1"/>
#</routes>
