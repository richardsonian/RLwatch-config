var backupSchedule = {
    "name": "20 Minute Hall",
    "periods": [
        {
            "period": 0,
            "name": "Homeroom",
            "start": "8:15",
            "end": "8:20"
        },
        {
            "period": 0,
            "name": "Hall",
            "start": "8:25",
            "end": "8:45"
        },
        {
            "period": 1,
            "name": "1st",
            "start": "8:50",
            "end": "9:35",
            "block": "G"
        },
        {
            "period": 2,
            "name": "2nd",
            "start": "9:40",
            "end": "10:25",
            "block": "H"
        },
        {
            "period": 3,
            "name": "3rd",
            "start": "10:30",
            "end": "11:15",
            "block": "A"
        },
        {
            "period": 4,
            "name": "4th - First Lunch",
            "start": "11:20",
            "end": "11:45",
            "block": "B"
        },
        {
            "period": 4,
            "name": "4th - In Between Lunches",
            "start": "11:50",
            "end": "12:05",
            "block": "B"
        },
        {
            "period": 4,
            "name": "4th - Second Lunch",
            "start": "12:05",
            "end": "12:30",
            "block": "B"
        },
        {
            "period": 5,
            "name": "5th",
            "start": "12:35",
            "end": "13:20",
            "block": "C"
        },
        {
            "period": 6,
            "name": "6th",
            "start": "13:25",
            "end": "14:10",
            "block": "D"
        }/*,
        {
            "period": 7,
            "name": "7th",
            "start": "14:15",
            "end": "15:00",
            "block": "E"
        }*/
    ]
};
function xhrRequest(url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function() {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
}
function getDaySchedule() {
  xhrRequest('http://casper.roxburylatin.org/todays_schedule.json', 'GET', function(responseText) {
    //var schedule = backupSchedule;
    var schedule = JSON.parse(responseText);
    
    var array = ["", "", "", "", "", "", "", "", "", "", ""];
    
    if(schedule.periods){
      for(var i = 0; i<schedule.periods.length; i++) {
        if(!schedule.periods[i].block) {schedule.periods[i].block = 'Z';}
        for(var j in schedule.periods[i]) {
          if(j=="period") {array[i]=schedule.periods[i][j];}
          else {array[i]+=":"+schedule.periods[i][j];}
        }
      }
    }
    
    var scheduleDict = {};
    scheduleDict.PURPOSE = 0; //0 for schedule
    scheduleDict.PERIOD_ZERO = array[0] || "noperiod";
    scheduleDict.PERIOD_ONE = array[1] || "noperiod";
    scheduleDict.PERIOD_TWO = array[2] || "noperiod";
    scheduleDict.PERIOD_THREE = array[3] || "noperiod";
    scheduleDict.PERIOD_FOUR = array[4] || "noperiod";
    scheduleDict.PERIOD_FIVE = array[5] || "noperiod";
    scheduleDict.PERIOD_SIX =  array[6] || "noperiod";
    scheduleDict.PERIOD_SEVEN =  array[7] || "noperiod";
    scheduleDict.PERIOD_EIGHT =  array[8] || "noperiod";
    scheduleDict.PERIOD_NINE = array[9] || "noperiod";
    scheduleDict.PERIOD_TEN =  array[10] || "noperiod";
    
    
    Pebble.sendAppMessage(scheduleDict, function() {
      //console.log('Message sent successfully: ' + JSON.stringify(scheduleDict));
    }, function(e) {
      //console.log('Message failed: ' + JSON.stringify(e));
    });
  });
}

var readyDict = {
  "PURPOSE":1
};

Pebble.addEventListener('showConfiguration', function() {
  console.log("Config opened");
  var url = 'https://richardsonian.github.io/RLwatch-config/config.html';
  Pebble.openURL(url);
});
Pebble.addEventListener('webviewclosed', function(e) {
  console.log("config closed");
  var configData = JSON.parse(decodeURIComponent(e.response));
  
  var dict = {
    'PURPOSE': 2,
    'CLASS_A': configData.ABlockClasses,
    'CLASS_B': configData.BBlockClasses,
    'CLASS_C': configData.CBlockClasses,
    'CLASS_D': configData.DBlockClasses,
    'CLASS_E': configData.EBlockClasses,
    'CLASS_F': configData.FBlockClasses,
    'CLASS_G': configData.GBlockClasses,
    'CLASS_H': configData.HBlockClasses
  };
  
  Pebble.sendAppMessage(dict, function() {
    //console.log('Config data sent successfully!');
  }, function(e) {
    //console.log('Error sending config data!');
  });
});


Pebble.addEventListener('ready', function(e) {
  //console.log('PebbleKit JS ready!');
  Pebble.sendAppMessage(readyDict, function(){
    //console.log('Message sent successfully: ' + JSON.stringify(readyDict));                    
  }, function(e) {
    //console.log('Message failed: ' + JSON.stringify(e));
  });
});

Pebble.addEventListener('appmessage', function(msg) {
  //console.log('AppMessage received! Sending schedule...');
  //var message = msg.payload;
  getDaySchedule();
});

