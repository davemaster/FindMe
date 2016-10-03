//FindMe System
//FindMe_Base.js
//Programmer : David E. Flores Escalante
//Project: FindMe System for Artik Cloud
//Date: Sep 2016
//Description: This listen the serial port for data which indicates an RF Signal was received by the PIC16F628A

var webSocketUrl = "wss://api.artik.cloud/v1.1/websocket?ack=true";
var device_id = "7882fde5d66d4543970f83254fab4a04";
var device_token = "b86c190fe2cf427aa7e5b99019ea750a";

var WebSocket = require('ws');
var isWebSocketReady = false;
var ws = null;

var gpio = require('rpi-gpio');
var myPin = 13;			// LED RF signal received indicator*/
var myFindMeState = 0;

var SerialPort = require("serialport");

var port = new SerialPort("/dev/ttyAMA0", {
  baudRate: 9600
});

//var sleep = require('sleep');

/**
 * Gets the current time in millis
 */
function getTimeMillis(){
    return parseInt(Date.now().toString());
}

/**
 * Create a /websocket connection and setup GPIO pin
 */
function start() {
	
	console.log("\nFindMe System - Base");
	
	
    //Create the WebSocket connection
    isWebSocketReady = false;
    ws = new WebSocket(webSocketUrl);
    ws.on('open', function() {
        console.log("\nWebSocket connection is open ....");
        register();
    });
    
    ws.on('close', function() {
        console.log("WebSocket connection is closed ....");
	exitClosePins();
    });
	
	port.on('open', function(err) {
		if (err) {
		  return console.log('port not open', err.message);
		}
		console.log('port open');
	});
	
	port.on('data', function (data) {
		//console.log('Data: ' + data);
		
		
		var newState=0;
		var i = data.indexOf("ON"); // Split data 
		//console.log(i);
		
		if(i>0)
		{
			console.log('myRFButton ON\n');
			console.log('SmartBag Missing...\n');
		
			newState=1;
			toggleFindMeLED(newState);
			
		}	
		else
		{
			i = data.indexOf("OFF"); // Split data 
			if(i>0)
			{
				console.log('myRFButton OFF\n');
				console.log('SmartBag founded...\n');
		
				newState=0;
				toggleFindMeLED(newState);
				
			}	
		}
		
	});


    gpio.setup(myPin, gpio.DIR_OUT, function(err) {
        if (err) throw err;
        myFindMeState = false; // default to false after setting up
        console.log('Setting pin ' + myPin + ' to out succeeded! \n');
     });
	 
}

/**
 * Sends a register message to /websocket endpoint
 */
function register(){
    console.log("Registering device on the WebSocket connection");
    try{
        var registerMessage = '{"type":"register", "sdid":"'+device_id+'", "Authorization":"bearer '+device_token+'", "cid":"'+getTimeMillis()+'"}';
        //console.log('Sending register message ' + registerMessage + '\n');
		console.log('Sending register message to Artik Cloud\n');
        ws.send(registerMessage, {mask: true});
        isWebSocketReady = true;
    }
    catch (e) {
        console.error('Failed to register messages. Error in registering message: ' + e.toString());
    }    
}

function toggleFindMeLED(value) {
   gpio.write(myPin, value, function(err) {
        if (err) throw err;
        myFindMeState = value;
        console.log('toggleLED: wrote ' + value + ' to pin #' + myPin);
        sendStateToArtikCloud();
    });

}

/**
 * Send one message to ARTIK Cloud
 */
function sendStateToArtikCloud(){
    try{
        ts = ', "ts": '+getTimeMillis();
        var data = {
              "STATE": myFindMeState
            };
        var payload = '{"sdid":"'+device_id+'"'+ts+', "data": '+JSON.stringify(data)+', "cid":"'+getTimeMillis()+'"}';
        //console.log('Sending payload ' + payload + '\n');
		// aaa   console.log('Sending STATE=ON to Artik Cloud...\n');
        ws.send(payload, {mask: true});
		console.log('Sending STATE=ON to Artik Cloud...\n');
    } catch (e) {
        console.error('Error in sending a message: ' + e.toString() +'\n');
    }    
}

/** 
 * Properly cleanup the pins
 */
function exitClosePins() {
    gpio.destroy(function() {
        console.log('Exit and destroy all pins!');
        process.exit();
    });
}

/**
 * All start here
 */

start();

process.on('SIGINT', exitClosePins);


