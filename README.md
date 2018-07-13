# Pozyx_FDI
Pozyx usage tests tailored for usage in 411's office of the Facultad de Informática of Universidad Complutense de Madrid. 

Initial configuration was made using Pozyx's cloud platform with auto calibrate function. We obtained the next map and ranges using max transmission power:
![Online map](images/cloudMap.png)

## Multitag positioning
Based on the [tutorial 4](https://www.pozyx.io/Documentation/Tutorials/multitag_positioning) from Pozyx's website.
Once the anchors have been positioned burn the .ino to a compatible board that has a Pozyx tag connected. Start the .pde with Processing to begin the tracking.

![Offline map](images/offlineMap.png)

---

## Max range 
The aim of this experiment is to test the maximum distance at which two given tags can communicate through UltraWideBand.

For this experiment the base boards are two SAMD21 based [Adafruit METRO M0 Express.](https://www.adafruit.com/product/3505)

---

Enjoy.
