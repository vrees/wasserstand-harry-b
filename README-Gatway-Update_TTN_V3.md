  # Update from TTN V2 to TTN V3


### Firmware
https://downloads.rakwireless.com/LoRa/RAK2245-Pi-HAT/Firmware/RAK2245_Latest_Firmware.zip
https://docs.rakwireless.com/Product-Categories/WisLink/RAK2245-Pi-HAT/Quickstart/#prerequisites
https://downloads.rakwireless.com/en/LoRa/RAK2245-Pi-HAT/Hardware-Specification/RAK2245_RAK831_Design_Guide_Comparison.pdf


### Was necessary to reset SX1301 chip by using Pin GPIO 25 because my board from Charles Hallard uses Pin 25 for reset of SX1301

```
sudo vi /opt/ttn-gateway/packet_forwarder/lora_pkt_fwd/start.sh

change 
   SX1301_RESET_BCM_PIN=17
to
   SX1301_RESET_BCM_PIN=25   
```

Schema of the board from Charles Hallard: https://raw.githubusercontent.com/hallard/RAK831-Zero/master/pictures/PiZero-RAK-sch.png

```
cd /opt/ttn-gateway/lora_gateway
./reset_lgw.sh start 25
./reset_lgw.sh stop 25

sudo journalctl -f
```
