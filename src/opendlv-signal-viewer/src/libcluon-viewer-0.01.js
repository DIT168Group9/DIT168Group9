// "Instantiate" libcluon.
let __libcluon = libcluon();
let pedalPosition;
let groundSteering;
let distanceReading;
let announcePresenceVehicalIp;
let announcePresenceGroupId;
let followerRequest;
let followResponse;
let stopFollow;
let followerStatus;
let leaderSpeed;
let leaderSteering;
let temperatureReading;

// Function to load data from a remote destination.
function getResourceFrom(url) {
    var xmlHttp = new XMLHttpRequest();
    xmlHttp.open("GET", url, false /*asynchronous request*/);
    xmlHttp.send(null);
    return xmlHttp.responseText;
}

// Check for availability of WebSockets.
if ("WebSocket" in window) {

    // Read data in binary from the WebSocket, channel "od4".
    var ws = new WebSocket("ws://" + window.location.host + "/", "od4");
    ws.binaryType = 'arraybuffer';

    ws.onopen = function() {
        console.log("Connected.");
        // Load the ODVD message specification file to have information about the messages to decode and encode.
        var odvdMessageSpecificationFile = getResourceFrom("opendlv-standard-message-set-v0.9.4.odvd");
        console.log("Loaded " + __libcluon.setMessageSpecification(odvdMessageSpecificationFile) + " messages from specification.");
    };

    ws.onmessage = function(evt) {

        // Got new data from the WebSocket; now, try to decode it into JSON using the supplied message specification file.
        var data = JSON.parse(__libcluon.decodeEnvelopeToJSON(evt.data));
        // console.log(data);

        // animateDashboard(data);

        if (data.dataType === 1041) {
            pedalPosition = (data.opendlv_proxy_PedalPositionReading.position) * 100;
            if(pedalPosition < 0){
                pedalPosition = pedalPosition * -1;
                $("#rpm").data("kendoRadialGauge").value(pedalPosition);
            }else{
                $("#rpm").data("kendoRadialGauge").value(pedalPosition);
            }
        }
        else if(data.dataType === 1045){
            groundSteering = (data.opendlv_proxy_GroundSteeringReading.groundSteering) * 100 + 16;
            if(groundSteering < 0){
                // groundSteering = groundSteering * -1;
                $("#kmh").data("kendoRadialGauge").value(groundSteering);
            }else{
                $("#kmh").data("kendoRadialGauge").value(groundSteering);
            }
        }
        else if(data.dataType === 1039){
            distanceReading = (data.opendlv_proxy_DistanceReading.distance) * 800;

            $(".meter > span").each(function() {
                $(this)
                    .data("origWidth", $(this).width())
                    .width(distanceReading)
                    .animate({
                        width: distanceReading
                    }, 'fast');
            });

            $("#fuel").data("kendoRadialGauge").value(distanceReading);
        }

        else if(data.dataType === 1035){
            temperatureReading = data.opendlv_proxy_TemperatureReading.temperature;
            $("#water-temprature").data("kendoRadialGauge").value(temperatureReading);
        }

        else if(data.dataType === 1001){
            announcePresenceVehicalIp = window.atob(data.AnnouncePresence.vehicleIp);
            announcePresenceGroupId = window.atob(data.AnnouncePresence.groupId);

            $("#apList").append('<li>' + " Group " + announcePresenceGroupId + " has the ip: " + announcePresenceVehicalIp + '</li>');
        }

        else if(data.dataType === 2001){
            leaderSpeed = data.LeaderStatus.speed;
            leaderSteering = data.LeaderStatus.steeringAngle;
            $("#apList").append('<li>' + " Leader's speed value is: " + leaderSpeed + " And the steering value is: " + leaderSteering + '</li>');
        }

        else if(data.dataType === 1002){
            followerRequest = data.FollowRequest.temporaryValue;
            $("#apList").append('<li>' + " Follow Request Received! " + '</li>');
        }

        else if(data.dataType === 1003){
            followResponse = data.FollowRequest.temporaryValue;
            $("#apList").append('<li>' + " Follow Response Received! " + '</li>');
        }

        else if(data.dataType === 1004){
            stopFollow = data.FollowRequest.temporaryValue;
            $("#apList").append('<li>' + " Stop Follow Received! " + '</li>');
        }

        else if(data.dataType === 3001){
            followerStatus = data.FollowRequest.temporaryValue;
            $("#apList").append('<li>' + " Follower Status Received! " + '</li>');
        }

    };
    ws.onclose = function() {
        console.log("Connection is closed.");
    };
}
else {
    console.log("WebSocket is not supported by your Browserb!");
}

/*
* Kendo Car Dashboard logic
 */

function createDashboard() {
    $("#rpm").kendoRadialGauge({
        theme: "black",

        pointer: {
            value: 0,
            color: "#ea7001"
        },

        scale: {
            startAngle: -45,
            endAngle: 120,

            min: 0,
            max: 50,

            majorUnit: 10,
            majorTicks: {
                width: 1,
                size: 7
            },

            minorUnit: 2,
            minorTicks: {
                size: 5
            },

            ranges: [{
                from: 30,
                to: 40,
                color: "#ff7a00"
            }, {
                from: 40,
                to: 50,
                color: "#c20000"
            }],

            labels: {
                font: "11px Arial,Helvetica,sans-serif"
            }
        }
    });

    $("#kmh").kendoRadialGauge({
        theme: "black",

        pointer: {
            value: 0,
            color: "#ea7001"
        },

        scale: {
            // startAngle: -90,
            // endAngle: 270,

            startAngle: 250,
            endAngle: -70,

            min: -100,
            max: 100,

            majorTicks: {
                width: 1,
                size: 14
            },
            majorUnit: 20,

            minorTicks: {
                size: 10
            },

            minorUnit: 5,

            // visible: false
        }
    });

    $("#fuel").kendoRadialGauge({
        theme: "black",

        pointer: {
            value: 0,
            color: "#ea7001"
        },

        scale: {
            startAngle: 90,
            endAngle: 180,

            min: 0,
            max: 50,

            majorUnit: 25,
            majorTicks: {
                width: 2,
                size: 6
            },

            minorUnit: 5,
            minorTicks: {
                size: 3
            },

            ranges: [{
                from: 0,
                to: 10,
                color: "#c20000"
            }],

            labels: {
                font: "9px Arial,Helvetica,sans-serif"
            }
        }
    });

    $("#water-temprature").kendoRadialGauge({
        theme: "black",

        pointer: {
            value: 0,
            color: "#ea7001"
        },

        scale: {
            startAngle: 180,
            endAngle: 270,

            min: 0,
            max: 90,

            majorUnit: 30,
            majorTicks: {
                width: 2,
                size: 6
            },

            minorUnit: 10,
            minorTicks: {
                size: 3
            },

            ranges: [{
                from: 70,
                to: 90,
                color: "#c20000"
            }],

            labels: {
                font: "9px Arial,Helvetica,sans-serif"
            }
        }
    });
}



$(document).ready(function() {

    createDashboard();


    $(document).bind("kendo:skinChange", function(e) {
        createDashboard();
    });

});