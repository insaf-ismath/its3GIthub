import xml.etree.cElementTree as ET
import random
from random import randint

# Mean time between vehicles
lambd0 = 2#seconds  #lane0 
lambd1 = 2#seconds  #lane1

# print("Lambda 0: " + str(lambd0))
# print("Lambda 1: " + str(lambd1))

# Number of vehicels in one lane
num = 100

currentTime1 = random.expovariate(1.0/lambd1)
currentTime0 = random.expovariate(1.0/lambd0)

vipDepartureTime = random.uniform(max(lambd1,lambd0)*num/4,max(lambd1,lambd0)*num/2)
# print("VIP departure time: " + str(vipDepartureTime))
#add DOM elements
root = ET.Element("routes")
ET.SubElement(root, "vType", tau="0", accel="2.9", decel="7.5", id="Car", length="4.3", maxSpeed="27.78", sigma="0.0", minGap="-1").text = ""        #for normal vehicles

ET.SubElement(root, "vType", tau="0", accel="2.9", decel="7.5", id="CarVIP", length="6.5", maxSpeed="27.78", sigma="0.0", minGap="-1").text = ""    #for VIP

ET.SubElement(root, "route", id="route0", edges="1to2").text = ""


print("Starting rou file generation")

loop_runner0 = False
loop_runner1 = False
loop_runner = True
lane0_count = 0
lane1_count = 0
while(True):
    if(currentTime0 < currentTime1 and (not loop_runner0)):
        
        if lane0_count != num/2:
            ET.SubElement(root, "vehicle", color="green", depart=str(currentTime1), departLane="0", arrivalLane="0", id=str(lane1_count+lane0_count), route="route0", type="Car").text = ""
        else:
            ET.SubElement(root, "vehicle", color="red", depart=str(currentTime1), departLane="0", arrivalLane="0", id="vip", route="route0", type="CarVIP").text = ""
            print("VIP time: " + str(currentTime1))
        currentTime0 = currentTime0 + random.expovariate(1.0/lambd1)
        lane0_count += 1
        print("Lane0 count: " + str(lane0_count))
        if(lane0_count == num): 
            loop_runner0 = True

    elif not loop_runner1:
        ET.SubElement(root, "vehicle", color="green", depart=str(currentTime1), departLane="1", arrivalLane="1", id=str(lane1_count+lane0_count), route="route0", type="Car").text = ""
        currentTime1 = currentTime1 + random.expovariate(1.0/lambd1)
        lane1_count += 1
        if(lane1_count == num): 
            loop_runner1 = True
        print("Lane1 count: " + str(lane1_count))

    if(loop_runner0 and loop_runner1): 
        break
    
    

print("Done generation!!!\nWriting XML file")
tree = ET.ElementTree(root)
tree.write("hello.rou.xml")
print("XML file writing done!!!")


#<routes>
#<vType tau="0" accel="1.0" decel="2.0" id="Car" length="2.0" maxSpeed="10.0" sigma="0.0" minGap="-1"/>
#<vType tau="0" accel="10.0" decel="10.0" id="Car1" length="2.0" maxSpeed="180.0" sigma="0.0" minGap="-1"/>
#<route id="route0" edges="1to2"/>
#<flow id="1" begin="0" end="50" depart="0" type="Car" period="2.5" departLane="1" arrivalLane="1" route="route0"/>
#<flow id="2" begin="4" end="54" depart="0" type="Car" period="2.5" departLane="0" arrivalLane="0" route="route0"/>
#<vehicle color="red" depart="55" departLane="1" arrivalLane="1" id="vip" route="route0" type="Car1"/>
#</routes>
