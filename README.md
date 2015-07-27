# Connecting sensors to the cloud

I've connected a [temperature & humidity sensor][dht], a photoresistor and a LED to an&nbsp;[Ethernet shield][eth] plugged onto my Arduino Uno.

![Schematic](https://github.com/mbialon/microcoap/blob/master/doc/schematic.png)

I've used a [CoAP][coap] [library][microcoap] and I've created an [Arduino][arduino] [sketch][sketch].
It connects to the internet and sends sensor readings to the [cloud][teo].

I may view them in the application or on a dashboard.

![Dashboard](https://github.com/mbialon/microcoap/blob/master/doc/dashboard.png)

[coap]: http://coap.technology
[arduino]: http://arduino.cc
[teo]: https://telemetria-online.pl/en/#app
[eth]: http://www.dx.com/p/ethernet-shield-with-wiznet-w5100-ethernet-chip-tf-slot-118061#.VYU-tWBVuHo
[dht]: http://www.dx.com/p/keyes-dht22-fr4-temperature-humidity-sensor-module-for-arduino-red-white-300285#.VYVPbmBVuHo
[microcoap]: https://github.com/1248/microcoap
[sketch]: https://github.com/mbialon/microcoap/blob/master/microcoap.ino
[repo]: https://github.com/mbialon/microcoap
