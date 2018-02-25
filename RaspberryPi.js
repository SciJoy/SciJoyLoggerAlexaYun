

var awsIot = require('aws-iot-device-sdk');
var SerialPort = require('serialport');
var port = new SerialPort('/dev/ttyACM0');

var thingShadows = awsIot.thingShadow({
   keyPath: "/home/pi/certs/sjpriv.pem.key",
  certPath: "/home/pi/certs/sjcert.pem.crt",
    caPath: "/home/pi/certs/aws-iot-rootCA.crt",
  clientId: "",
      host: ""
});


var clientTokenUpdate;

      thingShadows.on('connect', function() {
        	      console.log('connected to AWS IoT');

      thingShadows.register( 'SciJoyYun3Thing', {}, function() {

       var lightState = {"state": {"reported":{"welcome":"off"}}}
       clientTokenUpdate = thingShadows.update('SciJoyYun3Thing', lightState);

});

   });

//var stateObject ={};

    thingShadows.on('delta',
        function(thingName, stateObject) {   
	console.log('received delta on '+thingName+': '+
                       JSON.stringify(stateObject));
	
	thingShadows.update(thingName, {
	state: {
		reported: stateObject.state
	}
	
	//console.log('this be the shadow: ' + deltaShadow);
	        
	});

var deltaSensor = stateObject.state.Sensor;
var deltaSampleRate = stateObject.state.SampleRate;
var deltaSensorDuration = stateObject.state.SensorDuration;
var deltaThresholdMM = stateObject.state.ThresholdMM;
var deltaThreshold = stateObject.state.Threshold;
var deltaActuator = stateObject.state.Actuator;
var deltaActuatorDuration = stateObject.state.ActuatorDuration;
var deltaTriggerMM = stateObject.state.TriggerMM;
var deltaTrigger = stateObject.state.Trigger;

var shadowforSerial = deltaSensor; 
shadowforSerial+=",";
shadowforSerial += deltaSampleRate; 
shadowforSerial+=",";
shadowforSerial+= deltaSensorDuration;
shadowforSerial+=",";
shadowforSerial+= deltaThresholdMM;
shadowforSerial+=",";
shadowforSerial+= deltaThreshold;
shadowforSerial+=",";
shadowforSerial+= deltaActuator;
shadowforSerial+=",";
shadowforSerial+= deltaActuatorDuration;
shadowforSerial+=",";
shadowforSerial+= deltaTriggerMM;
shadowforSerial+=",";
shadowforSerial+= deltaTrigger;

console.log(shadowforSerial);

port.write('main screen turn on', function(err) {
if (err) {
return console.log('Error on write: ', err.message);
}
console.log('message written');
});

port.on('error', function(err){
console.log('Error: ', err.message);})


port.write(shadowforSerial);


}); 


//});


    thingShadows.on('timeout',
        function(thingName, clientToken) {
           console.log('received timeout on '+thingName+
                       ' with token: '+ clientToken);}); 
