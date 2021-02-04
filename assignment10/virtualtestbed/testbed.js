var PythonShell = require('python-shell');

const SerialPort = require('serialport');
const ConsoleReader = require('readline');
const Mqtt = require('mqtt');
const kafka = require('kafka-node');

//const client = new kafka.KafkaClient({kafkaHost: '138.246.232.197:9092'});

const Readline = SerialPort.parsers.Readline;
const parser = new Readline();

var first=true;

var myArgs=process.argv.slice(2);
console.log('myArgs: ', myArgs);
if (myArgs.length==0){
	console.log("Error: serial port not specified.");
	process.abort();
}

if (myArgs.length>2){
	console.log("Error: too many arguments");
	process.abort();
}

const port = new SerialPort(myArgs[0]);

const rl = ConsoleReader.createInterface({
  input: process.stdin,
  prompt: "hallo>"
});


try{
	port.pipe(parser);
	parser.on('data', handleDeviceString);
}
catch(e) {
  console.log(e);
  process.abort();
}

//handle input from terminal
rl.on('line', (input) => {
  console.log(`Received: ${input}`);
  input=input+"\n";
  port.write(input);
});

// const Consumer = kafka.Consumer;
// const consumer = new Consumer(
//     client,
//         [
//             { topic: '01_06_020'}
//         ],
//         {
//             autoCommit: false
//         }
//     );
 
// consumer.on('message', function (message) {
// 	message=message+"\n";
// 	port.write(message);
// 	console.log(`Received: ${message.value.toString()}`)
//})

//If only the serial port is given, send current time only.
//If a file name is given, open file and read the commands and set he time to the time of the first command - 30 seconds
if (myArgs.length==2){
	//read and process file given in myArgs[1]
	processFile(myArgs[1],port);
	sendCurrentTime(port);
} else {
	sendCurrentTime(port);
}

let timer = setInterval(function() {
	var { PythonShell } = require('python-shell');

	let options = {
		mode: 'text'
	};

	PythonShell.run('./forcaster.py', options, function (err, results) {
		if (err) throw err;
		// results is an array consisting of messages collected during execution
		console.log(results[0]);
	});
}, 1000*10*1); // time is in milliseconds. 1000 ms * 60 sec * 2 min


//Handles lines printed in device to serial connection
//Echos all lines and handles SEND commands by pushing them to the MQTT gateway of the IoT platform
function handleDeviceString(str){
  console.log('>>> '+str);
  if(str.startsWith("<<SEND:")){

	var serialData = str.substring(7, str.length-3).split(":")

    var payload = {
		username:"group4_2020_ws",
		sensor_matthias: serialData[1],
		device_id: "76",
		timestamp: serialData[0]+"00"
    };

	var options = {
	  host: '131.159.35.132',
	  port: 1883,
	  username: 'JWT',
	  password: 'eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9.eyJpYXQiOjE2MDgyMjcyMjYsImlzcyI6ImlvdHBsYXRmb3JtIiwic3ViIjoiMjVfNzYifQ.I8QPuYCj9JfVSH62stk0vDWDX4a_Duc_tMBM34oKhztpAy6EGJckVF6Ekef-zjLJtrrTbHlUa9NRj-WngSV0suU-DM07EGERz05TS1yGd7wWGu9JRzz4kR0XFdU93UnpUpQs1bVTVWy92UILu1V0MGWY6NXKod9eTGPZCk7zd2NRZG91_0vg_9N0nesDjhQB03_-5wDUQ1L233TIvWu_35_JNisDFfU_kKVwt-CYXZcRdmcD0X4B_D8VF0RstCyR91sTzQC2VDmkIYlZw6mYd2kc9oarNO5PjUJVhPDqdML5hk06TOVU68WMIMRipzgCiaauEr06ixVitH3yFCXzUlGvdLyN3Dg2LXLcDb-Cbvs1OjFG2JNc8K3WDALsLKanKkIcAflG05S--KckiMaSxqUXov0cT51SjsZnUFzi8iDsfFcnHmcrseqkaxF_AuD8UfqIu9-k1khOICg2y3jXoIK_HXKRbDSFeTe36uTCXZpVYqj21M0eoFA-3NwrcnfXRH8aTGl74L0UXAmCpD9CNS9DOPdh7mrRzxgerU1N3JLR5Qk_2yzVaI18ImvfLTz2hFrqI2jjHgB3LKv9cPu7gBclIFEiqffvg4IyHpm6ZGzuZlbpmymPh0iLN0gns4f0Lk-Q43xA2oEkaCJd0nFF_sJzN6OelaOPteeekIBJuKk',
	  clean: true
	}

	// var client = Mqtt.connect(options)
	//
	// client.on('connect', function() {
	// 	console.log('MQTT client connected');
	// 	client.publish('25_76', JSON.stringify(payload));
	// 	console.log("sent to mqtt");
	// });
  }
}

function sendCurrentTime(port) {
    var utc = new Date();

	var hours = utc.getHours();
	var minutes = utc.getMinutes();
	var seconds = utc.getSeconds();
	var day =utc.getDate();
	var month = utc.getMonth()+1;
	var year = utc.getFullYear();
  
  
	var tmp = utc.getTime()/1000;
	var time=Math.trunc(tmp)+7200;
	var timeString = "0:0:"+String(time)+"\n";

	port.write(timeString);
};

function processFile(fileName,port){
	var fs = require('fs');
	var buffer = fs.readFileSync(__dirname + "\\"+ fileName);
	port.write(buffer.toString());
}

process.on('uncaughtException', function (exception) {
	console.log(exception); // to see your exception details in the console
	// if you are on production, maybe you can send the exception details to your
	// email as well ?
});


