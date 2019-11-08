#Follower Kobuki BLE Implementation
==============

In terms of BLE communication, the Kobuki is a simple scanner. 

The leader Kobuki advertises manufacturer data (specified by the payload's type), which allows the follower Kobuki to filter through all received advertisements and find the desired payload. The advertisement event callback is implemented [here](https://github.com/connorgriffith/EECS149_Follow-The_Leader_Kobukis/blob/144d748381dd85e4e949d9f51b9875b34395b133/ble_adv_listen_template/main.c#L48). 

###As of 11/7/19:
The follower Kobuki can successfully receive the correct transmitted advertisements from the leader Kobuki.

