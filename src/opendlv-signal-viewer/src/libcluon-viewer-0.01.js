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
let gyroscopeReadingXAxis;
let gyroscopeReadingYAxis;
let gyroscopeReadingZAxis;

// Function to load data from a remote destination.
function getResourceFrom(url) {
    let xmlHttp = new XMLHttpRequest();
    xmlHttp.open("GET", url, false /*asynchronous request*/);
    xmlHttp.send(null);
    return xmlHttp.responseText;
}

// Check for availability of WebSockets.
if ("WebSocket" in window) {

    // Read data in binary from the WebSocket, channel "od4".
    let ws = new WebSocket("ws://" + window.location.host + "/", "od4");
    ws.binaryType = 'arraybuffer';

    ws.onopen = function() {
        console.log("Connected.");
        // Load the ODVD message specification file to have information about the messages to decode and encode.
        let odvdMessageSpecificationFile = getResourceFrom("opendlv-standard-message-set-v0.9.4.odvd");
        console.log("Loaded " + __libcluon.setMessageSpecification(odvdMessageSpecificationFile) + " messages from specification.");

        setInterval(onInterval, Math.round(1000 / g_renderFreq));
    };

    ws.onmessage = function(evt) {

        // Got new data from the WebSocket; now, try to decode it into JSON using the supplied message specification file.
        let data = JSON.parse(__libcluon.decodeEnvelopeToJSON(evt.data));

        // Pedal Position Readings

        if (data.dataType === 1041) {
            pedalPosition = (data.opendlv_proxy_PedalPositionReading.position) * 100;
            if(pedalPosition < 0){
                pedalPosition = pedalPosition * -1;
                $("#rpm").data("kendoRadialGauge").value(pedalPosition);
            }else{
                $("#rpm").data("kendoRadialGauge").value(pedalPosition);
            }
        }

        // Ground Steering Readings

        else if(data.dataType === 1045){
            groundSteering = (data.opendlv_proxy_GroundSteeringReading.groundSteering) * 100 + 16;
            if(groundSteering < 0){
                // groundSteering = groundSteering * -1;
                $("#kmh").data("kendoRadialGauge").value(groundSteering);
            }else{
                $("#kmh").data("kendoRadialGauge").value(groundSteering);
            }
        }

        // Ultrasonic Readings

        else if(data.dataType === 1039) {
            distanceReading = (data.opendlv_proxy_DistanceReading.distance) * 100;
            if (distanceReading <= 10 && distanceReading >= 1) {
                $(".stopSign").css('color', 'red');
            }
            else {
                $(".stopSign").css('color', '#1d2124');
            }

            if (distanceReading > 56) {
                $(".meter > span").each(function () {
                    $(this)
                        .data("origWidth", $(this).width())
                        .width(0)
                        .animate({
                            width: 280
                        }, 'fast');
                });
                $("#fuel").data("kendoRadialGauge").value(distanceReading);
            }

            else {

                $(".meter > span").each(function () {
                    $(this)
                        .data("origWidth", $(this).width())
                        .width(0)
                        .animate({
                            width: distanceReading * 5
                        }, 'fast');
                });

                $("#fuel").data("kendoRadialGauge").value(distanceReading);
            }
        }
        // Temperature Readings

        else if(data.dataType === 1035){
            temperatureReading = data.opendlv_proxy_TemperatureReading.temperature;
            $("#water-temprature").data("kendoRadialGauge").value(temperatureReading);
        }

        // Gyroscope Readings

        // else if(data.dataType === 1042){
        //     gyroscopeReadingXAxis = data.opendlv_proxy_GyroscopeReading.GyroscopeReadingX;
        //     gyroscopeReadingYAxis = data.opendlv_proxy_GyroscopeReading.GyroscopeReadingY;
        //     gyroscopeReadingZAxis = data.opendlv_proxy_GyroscopeReading.GyroscopeReadingZ;
        //
        // }

        // V2V messages in Signal Viewer

        else if(data.dataType === 1001){
            announcePresenceVehicalIp = window.atob(data.AnnouncePresence.vehicleIp);
            announcePresenceGroupId = window.atob(data.AnnouncePresence.groupId);

            $("#apList").append('<li>' + " Group " + announcePresenceGroupId + " has the ip: " + announcePresenceVehicalIp + '</li>');
            data.scrollTop = data.scrollHeight;
        }

        else if(data.dataType === 2001){
            leaderSpeed = data.LeaderStatus.speed;
            leaderSteering = data.LeaderStatus.steeringAngle;
            $("#apList").append('<li>' + " Leader's speed value is: " + leaderSpeed + " And the steering value is: " + leaderSteering + '</li>');
            $(".leaderSign").css('color', 'orangered');
        }

        else if(data.dataType === 1002){
            followerRequest = data.FollowRequest.temporaryValue;
            $("#apList").append('<li>' + " Follow Request Received! " + '</li>');
        }

        else if(data.dataType === 1003){
            followResponse = data.FollowRequest.temporaryValue;
            $("#apList").append('<li>' + " Follow Response Received! " + '</li>');
            $(".followSign").css('color', 'orangered');
        }

        else if(data.dataType === 1004){
            stopFollow = data.FollowRequest.temporaryValue;
            $("#apList").append('<li>' + " Stop Follow Received! " + '</li>');
            $(".leaderSign").css('color', '#1d2124');
            $(".followSign").css('color', '#1d2124');
        }

        else if(data.dataType === 3001){
            followerStatus = data.FollowRequest.temporaryValue;
            $("#apList").append('<li>' + " Follower Status Received! " + '</li>');
        }

        onMessageReceived(__libcluon, evt.data);

    };
    ws.onclose = function() {
        console.log("Connection is closed.");
    };

    $(document).ready(function() {


        $("#graphButton").on('click', function () {
            $("#goodOldSignal").toggle();

        });

        createDashboard();
        $(document).bind("kendo:skinChange", function(e) {
            createDashboard();
        });

    });

}
else {
    console.log("WebSocket is not supported by your Browser!");
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


const g_renderFreq = 4.0;
const g_maxDataAge = 10.0;
const g_maxFieldChartValues = 10;

var g_charts = new Map();
var g_chartConfigs = new Map();
var g_data = new Map();
var g_pause = false;


function onMessageReceived(lc, msg) {

    if (g_pause) {
        return;
    }

    var data_str = lc.decodeEnvelopeToJSON(msg);

    if (data_str.length === 2) {
        return;
    }

    d = JSON.parse(data_str);


    // Translate to nice JSON ..
    var payloadFields = new Array();

    const payloadName = Object.keys(d)[5];

    for (const fieldName in d[payloadName]) {
        const fieldValue = d[payloadName][fieldName];
        const field = {
            name : fieldName,
            value : fieldValue,
            type : (typeof fieldValue)
        };
        payloadFields.push(field);
    }

    const data = {
        dataType : d.dataType,
        payload : {
            name : payloadName,
            fields : payloadFields
        },
        received : d.received,
        sampleTimeStamp : d.sampleTimeStamp,
        senderStamp : d.senderStamp,
        sent : d.sent
    };
    // .. done.

    const sourceKey = data.dataType + '_' + data.senderStamp;
    const dataSourceIsKnown = g_data.has(sourceKey);

    if (!dataSourceIsKnown) {
        addTableData(sourceKey, data);
        addFieldCharts(sourceKey, data);

        g_data.set(sourceKey, new Array());
    }

    storeData(sourceKey, data);
}

function toTime(t) {
    const milliseconds = t.seconds * 1000 + t.microseconds / 1000;
    return moment(milliseconds).format('YYYY-MM-DD hh:mm:ss');
}

function cutLongField(type, value) {
    if (type === 'string' && value.length > 20) {
        value = value.substr(0, 20) + ' <span class="dots">[...]</span>';
    }
    return value;
}

function addTableData(sourceKey, data) {

    if($('tr#' + sourceKey).length === 0) {

        const name = data.payload.name;
        const type = data.dataType;
        const sender = data.senderStamp;
        const timestamp = toTime(data.sampleTimeStamp);

        const headerHtml = '<tr id="' + sourceKey + '" class="dataHeader"><td>'
            + type + '</td><td>' + sender + '</td><td>' + name + '</td><td id="'
            + sourceKey + '_frequency">N/A</td><td id="'
            + sourceKey + '_timestamp">' + timestamp + '</td></tr>';

        const fieldCount = data.payload.fields.length;

        var fieldsHtml = '<tr id="' + sourceKey + '_fields" class="hidden">'
            + '<td colspan="6"><table class="dataFields">';
        for (var i = 0; i < fieldCount; i++) {

            const field = data.payload.fields[i];
            const fieldName = field.name;
            const fieldValue = cutLongField(field.type, field.value);

            fieldsHtml += '<tr><td class="field-name">' + fieldName + '</td>'
                + '<td class="field-plot"><canvas id="' + sourceKey + '_field'
                + i + '_canvas"></canvas></td><td id="' + sourceKey + '_field'
                + i + '_value" class="field-value">' + fieldValue + '</td></tr>';
        }

        fieldsHtml += '</td></tr></table>';

        $('#dataView > tbody:last-child').append(headerHtml);
        $('#dataView > tbody:last-child').append(fieldsHtml);
    }
}

function addFieldCharts(sourceKey, data) {

    const fieldCount = data.payload.fields.length;

    for (var i = 0; i < fieldCount; i++) {

        const field = data.payload.fields[i];
        const fieldType = field.type;

        if (fieldType === 'number') {

            const fieldKey = sourceKey + '_' + i;

            const fieldName = field.name;
            const fieldValue = field.value;

            const config = {
                type: 'line',
                data: {
                    labels: [0.0],
                    datasets: [{
                        label: fieldName,
                        data: [fieldValue],
                        backgroundColor: 'rgb(255, 99, 132)',
                        borderColor: 'rgb(255, 99, 132)',
                        fill: false
                    }]
                },
                options: {
                    responsive: true,
                    title: {
                        display: false,
                    },
                    tooltips: {
                        mode: 'index',
                        intersect: false,
                    },
                    hover: {
                        mode: 'nearest',
                        intersect: true
                    },
                    legend: {
                        display: false
                    },
                    animation: {
                        duration: 0
                    },
                    scales: {
                        xAxes: [{
                            display: true,
                            scaleLabel: {
                                display: true,
                                labelString: 'time'
                            }
                        }],
                        yAxes: [{
                            display: true,
                            scaleLabel: {
                                display: true,
                                labelString: fieldName
                            }
                        }]
                    }
                }
            };

            var ctx = document.getElementById(sourceKey + '_field' + i + '_canvas').getContext('2d');
            var chart = new Chart(ctx, config);

            g_charts[fieldKey] = chart;
            g_chartConfigs[fieldKey] = config;
        }
    }
}

function storeData(sourceKey, data) {

    const newDataTime = data.sent.seconds;
    const dataList = g_data.get(sourceKey);

    var dataPointsToShift = 0;
    for (var i = 0; i < dataList.length; i++) {
        const prevData = dataList[i];
        const dataTime = prevData.sent.seconds;
        const age = newDataTime - dataTime;

        if (age > g_maxDataAge) {
            dataPointsToShift++;
        } else {
            break;
        }
    }

    for (var i = 0; i < dataPointsToShift; i++) {
        g_data.get(sourceKey).shift();
    }
    g_data.get(sourceKey).push(data);
}

function onInterval() {
    if (g_pause) {
        return;
    }

    g_data.forEach(function(dataList, sourceKey, map) {
        const newestData = dataList[dataList.length - 1];
        updateTableData(sourceKey, newestData);
        updateFieldCharts(sourceKey, dataList);
    });
}

function updateTableData(sourceKey, data) {

    const dataList = g_data.get(sourceKey);
    if (dataList.length > 10) {
        const firstTimestamp = dataList[0].sent;
        const firstTime = firstTimestamp.seconds * 1000 + firstTimestamp.microseconds / 1000;
        const lastTimestamp = dataList[dataList.length - 1].sent;
        const lastTime = lastTimestamp.seconds * 1000 + lastTimestamp.microseconds / 1000;
        const frequency = Math.round(1000 * dataList.length / (lastTime - firstTime));
        $('td#' + sourceKey + '_frequency').html(frequency);
    } else {
        $('td#' + sourceKey + '_frequency').html('N/A');
    }

    const timestamp = toTime(data.sampleTimeStamp);
    $('td#' + sourceKey + '_timestamp').html(timestamp);

    const fieldCount = data.payload.fields.length;
    for (var i = 0; i < fieldCount; i++) {
        const field = data.payload.fields[i];
        const fieldValue = cutLongField(field.type, field.value);
        $('td#' + sourceKey + '_field' + i + '_value').html(fieldValue);
    }
}

function updateFieldCharts(sourceKey, dataList) {

    const latestData = dataList[dataList.length - 1];
    const latestTimestamp = latestData.sent;
    const latestTime = latestTimestamp.seconds * 1000 + latestTimestamp.microseconds / 1000;

    const fieldCount = latestData.payload.fields.length;

    // Clear charts.
    for (var i = 0; i < fieldCount; i++) {
        const field = latestData.payload.fields[i];
        const fieldType = field.type;
        if (fieldType === 'number') {
            const fieldKey = sourceKey + '_' + i;
            const config = g_chartConfigs[fieldKey];
            config.data.labels = new Array();
            config.data.datasets[0].data = new Array();
        }
    }

    // Add new data.
    const dataCount = dataList.length;
    const decimation = Math.floor(dataCount / (g_maxFieldChartValues - 1));
    for (var j = 0, k = 0; j < dataCount; j++, k++) {
        if (j !== dataCount - 1) {
            if (k === decimation) {
                k = 0;
            }
            if (k !== 0) {
                continue;
            }
        }

        const data = dataList[j];
        const timestamp = data.sent;
        const time = timestamp.seconds * 1000 + timestamp.microseconds / 1000;

        const deltaTime = (time - latestTime) / 1000.0;

        for (var i = 0; i < fieldCount; i++) {
            const field = data.payload.fields[i];
            const fieldType = field.type;

            if (fieldType === 'number') {

                const fieldKey = sourceKey + '_' + i;
                const fieldValue = field.value;

                const config = g_chartConfigs[fieldKey];

                config.data.labels.push(deltaTime.toFixed(1));
                config.data.datasets[0].data.push(fieldValue);
            }
        }
    }

    // Update.
    for (var i = 0; i < fieldCount; i++) {
        const field = latestData.payload.fields[i];
        const fieldType = field.type;
        if (fieldType === 'number') {
            const fieldKey = sourceKey + '_' + i;
            const chart = g_charts[fieldKey];
            chart.update();
        }
    }
}
